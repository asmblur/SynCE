/* $Id$ */
#include <synce_socket.h>
#include <synce_log.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include "dccm_config.h"
#if !HAVE_DAEMON
int daemon(int nochdir, int noclose);
#endif

#define DCCM_PID_FILE         "dccm.pid"
#define DCCM_PORT           	5679
#define DCCM_PING_INTERVAL   	5         /* seconds */
#define DCCM_PING            	0x12345678
#define DCCM_MAX_PING_COUNT   3					/* max number of pings without reply */
#define DCCM_MAX_PACKET_SIZE	512
#define DCCM_MIN_PACKET_SIZE	0x24

#define RESULT_SUCCESS 0
#define RESULT_FAILURE 1

typedef struct _Client
{
	SynceSocket* socket;
	struct sockaddr_in address;

  /* no lock, or unlocking successfull */
	bool authenticated;

  /* if we have received the initial zero package */
	bool initialized;

  /* for keep-alive */
	int ping_count;

  /* password stuff */
	bool locked;
	bool expect_password_reply;
	int key;

  /* set to disconnect client */
	bool disconnect;

  /* information package */
  bool has_information;
  uint16_t os_version;
  uint16_t build_number;
  uint16_t processor_type;
  uint32_t partner_id_1;
  uint32_t partner_id_2;
	char* name;
	char* class;
	char* hardware;

  /* is Windows CE 1.0 */
  bool is_windows_ce_10;
} Client;

static Client * current_client = NULL;
static bool running = true;
static bool is_daemon = true;
static bool exit_on_disconnect = false;
static bool allow_root = false;
static char* password = NULL;

static void disconnect_current_client()
{
	if (current_client)
		current_client->disconnect = true;
}

static void handle_sighup(int n)
{
	disconnect_current_client();
}

static void handle_sigterm(int n)
{
	running = false;
	disconnect_current_client();
}

/* Dump a block of data for debugging purposes */
static void
dump(desc, data, len)
	const char *desc;
	void *data;
	size_t len;
{
	uint8_t *buf = data;
	int i, j;
	char hex[8 * 3 + 1];
	char chr[8 + 1];

	printf("%s (%d bytes):\n", desc, len);
	for (i = 0; i < len + 7; i += 8) {
	    for (j = 0; j < 8; j++) 
		if (j + i >= len) {
			hex[3*j+0] = ' ';
			hex[3*j+1] = ' ';
			hex[3*j+2] = ' ';
			chr[j] = ' ';
		} else {
			uint8_t c = buf[j + i];
			const char *hexchr = "0123456789abcdef";
			hex[3*j+0] = hexchr[(c >> 4) & 0xf];
			hex[3*j+1] = hexchr[c & 0xf];
			hex[3*j+2] = ' ';
			if (c > ' ' && c <= '~')
				chr[j] = c;
			else
				chr[j] = '.';
		}
	    hex[8*3] = '\0';
	    chr[8] = '\0';
	    printf("  %04x: %s %s\n", i, hex, chr);
	}
}

/**
 * Write help message to stderr
 */
static void write_help(char *name)
{
	fprintf(
			stderr, 
			"Syntax:\n"
			"\n"
			"\t%s [-d LEVEL] [-f [-s]] [-h] [-p PASSWORD]\n"
			"\n"
			"\t-d LEVEL     Set debug log level\n"
			"\t                 0 - No logging\n"
			"\t                 1 - Errors only (default)\n"
			"\t                 2 - Errors and warnings\n"
			"\t                 3 - Everything\n"
			"\t-f           Do not run as daemon\n"
			"\t-s           Exit on device disconnect (only with -f)\n"
			"\t-h           Show this help message\n"
			"\t-p PASSWORD  Use this password when device connects\n"
      "\t-r           Allow dccm to run as root - USE AT YOUR OWN RISK!\n",
			name);
}

/**
 * Parse parameters to daemon
 */
static bool handle_parameters(int argc, char** argv)
{
	int c;
	int log_level = SYNCE_LOG_LEVEL_ERROR;

	while ((c = getopt(argc, argv, "d:fhp:rs")) != -1)
	{
		switch (c)
		{
			case 'd':
				log_level = atoi(optarg);
				break;
			
			/*
			 * The -f parameter specifies that we want to run in the foreground
			 */
			case 'f':
				is_daemon = false;
				break;

			case 'p':
				if (password)
					free(password);
				password = strdup(optarg);
				break;
				
			case 's':
				exit_on_disconnect = true;
				break;

      case 'r':
        allow_root = true;
        break;

			case 'h':
			default:
				write_help(argv[0]);
				return false;
		}
	}

	/* -s sanity check */
	if (is_daemon && exit_on_disconnect)
	{
		fprintf (stderr,
		         "The \'-s\' option only makes sense when running in foreground.\n\n");
		write_help(argv[0]);
		return false;
	}

	synce_log_set_level(log_level);

	return true;
}

/**
 * Begin listening on server port
 */
static SynceSocket* start_socket_server()
{
	SynceSocket* server = NULL;

	server = synce_socket_new();
	if (!server)
		goto error;

	if (!synce_socket_listen(server, NULL, DCCM_PORT))
		goto error;

	return server;

error:
	if (server)
		synce_socket_free(server);

	return NULL;
}

/*
 * Action is "connect" or "disconnect"
 */
static void run_scripts(char* action)
{
	char* directory = NULL;
	DIR* dir = NULL;
	struct dirent* entry = NULL;

	if (!synce_get_script_directory(&directory))
	{
		synce_error("Failed to get script directory");
		goto exit;
	}

	dir = opendir(directory);
	if (!dir)
	{
		synce_error("Failed to open script directory");
		goto exit;
	}

	while ((entry = readdir(dir)) != NULL)
	{
		char path[MAX_PATH];
		char command[MAX_PATH];
		struct stat info;
		
		snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);
		if (lstat(path, &info) < 0)
			continue;
		
		if (!(info.st_mode & S_IFREG))
			continue;

		synce_trace("Running command: %s %s", path, action);
		snprintf(command, sizeof(command), "%s %s", path, action);
		
		/* Run script */
		system(command);	
	}

exit:
	if (directory)
		free(directory);

	if (dir)
		closedir(dir);
}

/**
 * Look at an offset in a buffer for a string offset pointing to a string
 * in the same buffer.
 */
static char* string_at(unsigned char* buffer, size_t size, size_t offset)
{
	size_t string_offset = letoh32(*(uint32_t*)(buffer + offset));

	if (string_offset < size)
	{
		return wstr_to_ascii((WCHAR*)(buffer + string_offset));
	}
	else
	{
		synce_error("String offset too large: 0x%08x", string_offset);
		return NULL;
	}
}

static bool client_write_file(Client* client)
{
	bool success = false;
	char* filename = NULL;
	char *mode_str;
	mode_t file_mode;
	FILE* file = NULL;
	char ip_str[16];

	if (!inet_ntop(AF_INET, &client->address.sin_addr, ip_str, sizeof(ip_str)))
	{
		synce_error("inet_ntop failed");
		goto exit;
	}

	if (!synce_get_connection_filename(&filename))
	{
		synce_error("Unable to get connection filename");
		goto exit;
	}

	if ((file = fopen(filename, "w")) == NULL)
	{
		synce_error("Failed to open file for writing: %s", filename);
		goto exit;
	}

  fprintf(file,
      "# Modifications to this file will be lost next time a client connects to dccm\n"
      "\n"
      "[dccm]\n"
      "pid=%i\n"
      "\n"
      "[device]\n"
      "ip=%s\n"
      "port=%i\n"
      ,
      getpid(),
      ip_str,
      ntohs(client->address.sin_port));

  if (client->has_information)
    fprintf(file,
        "os_version=%i\n"
        "build_number=%i\n"
        "processor_type=%i\n"
        "partner_id_1=%i\n"
        "partner_id_2=%i\n"
        "name=%s\n"
        "class=%s\n"
        "hardware=%s\n"
        ,
        client->os_version,
        client->build_number,
        client->processor_type,
        client->partner_id_1,
        client->partner_id_2,
        client->name,
        client->class,
        client->hardware);

	if (client->locked)
	{
		fprintf(file,
				"password=%s\n"
				"key=%i\n",
				password,
				client->key
				);
	}

	success = true;
			
exit:
	if (file)
	{
		fclose(file);
		
		if ((mode_str = getenv ("SYNCE_CONNECTION_FILE_MODE")))
		{
			if (strlen (mode_str))
			{
				if (sscanf (mode_str, "%o", &file_mode) == 1)
				{
					chmod (filename, file_mode);
				}
			}
		}
	}
	if (filename)
		free(filename);
	return success;
}

static void remove_connection_file()
{
	char* filename = NULL;
	
	if (!synce_get_connection_filename(&filename))
	{
		synce_error("Unable to get connection filename");
		goto exit;
	}

	unlink(filename);

exit:
	if (filename)
		free(filename);
}


static bool client_read(Client* client)
{
	bool success = false;
	unsigned char* buffer = NULL;
	uint32_t header;

	if (!synce_socket_read(client->socket, &header, sizeof(header)))
	{
    if (client->initialized && !client->has_information)
    {
      // Windows CE 1.0 has closed the socket
      synce_info("Enabling special Windows CE 1.0 support");
      client->is_windows_ce_10 = true;
      success = true;
      client->disconnect = true;
    }
    else
		  synce_error("failed to read header");

		goto exit;
	}

	header = letoh32(header);
	/* synce_trace("Read header: 0x%08x", header); */

	if (0 == header)
	{
		/*
		 * Empty - ignore
		 */
		/* synce_trace("empty package"); */
    client->initialized = true;
	}
	else if (DCCM_PING == header)
	{
		/*
		 * This is a ping reply
		 */
		/* synce_trace("this is a ping reply"); */
		client->ping_count = 0;
	}
	else if (header < DCCM_MAX_PACKET_SIZE)
	{
		/*
		 * This is an information message
		 */
		/* synce_trace("this is an information message"); */

		if (header < DCCM_MIN_PACKET_SIZE)
		{
			synce_error("Packet is smaller than expected");
			goto exit;
		}

		buffer = malloc(header);

		if (!buffer)
		{
			synce_error("Failed to allocate %i (0x%08x) bytes", header, header);
			goto exit;
		}

		if (!synce_socket_read(client->socket, buffer, header))
		{
			synce_error("Failed to read package");
			goto exit;
		}

    client->has_information = true;

		dump("info package", buffer, header);

		/* Offset 0000 is always 24 00 00 00 ? */
		/* Offset 0004 contains the OS version, for example 03 00 = 3.0 */
    client->os_version = letoh16(*(uint16_t*)(buffer + 0x04));
		/* Offset 0006 contains the build number, for example 0x2ba3 = 11171 */
    client->build_number = letoh16(*(uint16_t*)(buffer + 0x06));
		/* Offset 0008 contains the processor type, for example 0x0a11 = 2577 */
    client->processor_type = letoh16(*(uint16_t*)(buffer + 0x08));
		/* Offset 000c is always 00 00 00 00 ? */

		/* Offset 0010 contains the first partner id */
    client->partner_id_1 = letoh32(*(uint32_t*)(buffer + 0x10));
		/* Offset 0014 contains the second partner id */
    client->partner_id_2 = letoh32(*(uint32_t*)(buffer + 0x14));

		client->name      = string_at(buffer, header, 0x18);
		client->class     = string_at(buffer, header, 0x1c);
		client->hardware  = string_at(buffer, header, 0x20);

    synce_info("Talking to '%s', a %s device of type %s",
        client->name, client->class, client->hardware);

		/*synce_trace("name    : %s", client->name);
		synce_trace("class   : %s", client->class);
		synce_trace("hardware: %s", client->hardware);*/

		free(buffer);
		buffer = NULL;

		if (client->locked)
		{
			client->expect_password_reply = true;
		}
		else
		{
      const uint32_t ping = htole32(DCCM_PING);
      if (!synce_socket_write(client->socket, &ping, sizeof(ping)))
			{
				synce_error("failed to send ping");
				goto exit;	
			}

			client->authenticated = true;
			client_write_file(client);
			run_scripts("connect");
		}
	}
	else
	{
		/*
		 * This is a password challenge
		 */

		client->key = header & 0xff;

		synce_trace("this is a password challenge");

    if (!password)
    {
      synce_error("A password is needed to connect to this device, but it was not supplied on the dccm command line.");
      goto exit;
    }

		if (!synce_password_send(client->socket, password, client->key))
		{
			synce_error("failed to send password");
			goto exit;
		}

		client->locked = true;
	}

	success = true;

exit:
	if (buffer)
		free(buffer);

	return success;
}

/**
 * Take care of a client
 */
static bool client_handler(Client* client)
{
	bool success = false;
	short events;
	const uint32_t ping = htole32(DCCM_PING);

	while (client->ping_count < DCCM_MAX_PING_COUNT && !client->disconnect)
	{
		/*
		 * Wait for event on socket
		 */
		events = EVENT_READ | EVENT_TIMEOUT;

		if (!synce_socket_wait(client->socket, DCCM_PING_INTERVAL, &events))
		{
			synce_error("synce_socket_wait failed");
			goto exit;	
		}

		/*synce_trace("got events 0x%08x", events);*/

		if (events & EVENT_READ)
		{
			if (client->expect_password_reply)
			{
				bool password_correct = false;

				if (!synce_password_recv_reply(client->socket, 2, &password_correct))
				{
					synce_error("failed to read password reply");
					goto exit;	
				}

        if (!password_correct)
        {
          synce_error("The password supplied on the dccm comand line was not correct!");
          goto exit;
        }

        if (!synce_socket_write(client->socket, &ping, sizeof(ping)))
        {
          synce_error("failed to send ping");
          goto exit;	
        }

				client->authenticated = true;
				client_write_file(client);
				run_scripts("connect");

				client->expect_password_reply = false;
			}
			else
			{
				if (!client_read(client))
				{
					synce_error("failed to read from client");
					goto exit;
				}
			}
		}
		else if (events & EVENT_TIMEOUT)
		{
			/* synce_trace("timeout event: sending ping"); */
			
			if (!synce_socket_write(client->socket, &ping, sizeof(ping)))
			{
				synce_error("failed to send ping");
				goto exit;	
			}

			client->ping_count++;
		}
    else if (events & EVENT_INTERRUPTED)
    {
      synce_trace("Connection interrupted");
      goto exit;
    }
		else
		{
			synce_error("unexpected events: %i", events);
			goto exit;
		}
	}

	synce_trace("Finished with client");

	success = true;

exit:
	return success;
}

bool write_pid_file(const char* filename)
{
	bool success = false;
	struct stat dummy;
	char pid_str[16];
	char *mode_str;
	mode_t file_mode;
	FILE* file = NULL;

	if (0 == stat(filename, &dummy))
	{
		/* File exists */
		file = fopen(filename, "r");
		
		if (!file)
		{
			synce_error("Failed to open %s for reading.", filename);
			goto exit;
		}

		if (fgets(pid_str, sizeof(pid_str), file))
		{
			pid_t pid = atoi(pid_str);
			if (0 == kill(pid, 0))
			{
				synce_error("It seems like dccm is already running with PID %i. If this is wrong, please remove the file %s and run dccm again.",
						pid, filename);
				goto exit;
			}
		}

		fclose(file);
		file = NULL;
	}

	file = fopen(filename, "w");

	if (!file)
	{
		synce_error("Failed to open %s for writing.", filename);
		goto exit;
	}

	snprintf(pid_str, sizeof(pid_str), "%i", getpid());

	fputs(pid_str, file);
	fclose(file);
	file = NULL;
	
	if ((mode_str = getenv ("SYNCE_PID_FILE_MODE")))
	{
		if (strlen (mode_str))
		{
			if (sscanf (mode_str, "%o", &file_mode) == 1)
			{
				chmod (filename, file_mode);
			}
		}
	}

	success = true;

exit:
	if (file)
		fclose(file);
	return success;
}

/**
 * Start here...
 */
int main(int argc, char** argv)
{
	int result = RESULT_FAILURE;
	SynceSocket* server = NULL;
	char* path = NULL;
	char pid_file[MAX_PATH];
	bool wrote_pid_file = false;

	umask(0077);
	
	if (!handle_parameters(argc, argv))
		goto exit;

  openlog("dccm", (is_daemon ? 0 : LOG_PERROR) | LOG_PID, LOG_DAEMON);
  synce_log_use_syslog();

	if (!allow_root && 0 == getuid())
	{
		synce_error("You should not run dccm as root. Terminating immediately.");
		goto exit;
	}

	if (!synce_get_directory(&path))
	{
		synce_error("Failed to get configuration directory name.");
		goto exit;
	}

	snprintf(pid_file, sizeof(pid_file), "%s/" DCCM_PID_FILE, path);
	free(path);

	if (password)
	{
		int i;
		char *p;

		/* Protect password */
		for (i = 0; i < argc; i++)
		{
			if (strcmp(argv[i], password) == 0)
			{
				p = argv[i];
				if (*p)
				{
					*p = 'X';
					p++;
					
					for (; *p; p++)
						*p = '\0';
				}
				break;
			}
		}
	}

	if (is_daemon)
	{
		synce_trace("Forking into background");
		daemon(0,0);
	}
	else
		synce_trace("Running in foreground");

	if (!write_pid_file(pid_file))
	{
		goto exit;
	}

	wrote_pid_file = true;

	/* signal handling */
	signal(SIGPIPE, SIG_IGN);
	signal(SIGHUP, handle_sighup);
	signal(SIGTERM, handle_sigterm);

	server = start_socket_server();
	if (!server)
	{
		synce_error("Failed to start socket server");
		goto exit;
	}
	
	remove_connection_file();
	run_scripts("start");
  
	synce_info("Listening for connections on port %i", DCCM_PORT);

	while (running)
	{
		/*
		 * Currently only handles one client...
		 */

		Client client;
    char ip_str[16];
    memset(&client, 0, sizeof(client));

		client.socket = synce_socket_accept(server, &client.address);
		if (!client.socket)
		{
			/* synce_error("Failed to accept client"); */
			goto exit;
		}

    if (!inet_ntop(AF_INET, &client.address.sin_addr, ip_str, sizeof(ip_str)))
    {
      synce_error("inet_ntop failed");
      goto exit;
    }

    synce_info("Connection from %s accepted", ip_str);

		current_client = &client;

		if (!client_handler(&client))
		{
			/* Better luck on next client? :-) */
		}
			
    synce_info("Connection from %s closed", ip_str);
		
		current_client = NULL;

		wstr_free_string(client.name);
		wstr_free_string(client.class);
		wstr_free_string(client.hardware);

		synce_socket_free(client.socket);

    if (client.is_windows_ce_10)
    {
			client_write_file(&client);
    }
    else
    {
      remove_connection_file();

      if (client.authenticated)
        run_scripts("disconnect");
    }
    
		if (exit_on_disconnect)
			running = false;

	}

	run_scripts("stop");
	result = RESULT_SUCCESS;
  
	synce_info("Shutting down.");

exit:
	if (password)
		free(password);

	synce_socket_free(server);

	if (wrote_pid_file)
		unlink(pid_file);

	return result;
}