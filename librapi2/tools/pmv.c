/* $Id$ */
#include "pcommon.h"
#include <rapi.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static void show_usage(const char* name)
{
	fprintf(stderr,
			"Syntax:\n"
			"\n"
			"\t%s [-h] SOURCE DESTINATION\n"
			"\n"
			"\t-h           Show this help message\n"
			"\tSOURCE       The current filename\n"
			"\tDESTINATION  The new filename\n",
			name);
}

static bool handle_parameters(int argc, char** argv, char** source, char** dest)
{
	int c;

	while ((c = getopt(argc, argv, "h")) != -1)
	{
		switch (c)
		{
			case 'h':
			default:
				show_usage(argv[0]);
				return false;
		}
	}

	if ((argc - optind) != 2)
	{
		fprintf(stderr, "%s: You need to specify source and destination file names on command line\n\n", argv[0]);
		show_usage(argv[0]);
		return false;
	}
		
	*source = strdup(argv[optind++]);
	*dest   = strdup(argv[optind++]);

	return true;
}

int main(int argc, char** argv)
{
	int result = 1;
	char* source = NULL;
	char* dest = NULL;
	BOOL success;
	HRESULT hr;
	WCHAR* wide_source = NULL;
	WCHAR* wide_dest = NULL;
	
	if (!handle_parameters(argc, argv, &source, &dest))
		goto exit;

	hr = CeRapiInit();

	if (FAILED(hr))
	{
		fprintf(stderr, "%s: Unable to initialize RAPI: %s\n", 
				argv[0],
				synce_strerror(hr));
		goto exit;
	}

	convert_to_backward_slashes(source);
	convert_to_backward_slashes(dest);

	wide_source = wstr_from_ascii(source);
	wide_dest   = wstr_from_ascii(dest);

	if (!CeMoveFile(wide_source, wide_dest))
	{
		fprintf(stderr, "%s: Cannot move '%s' to '%s': %s\n", 
				argv[0],
				source,
				dest,
				synce_strerror(CeGetLastError()));
		goto exit;
	}

	result = 0;

exit:
	wstr_free_string(wide_source);
	wstr_free_string(wide_dest);

	if (source)
		free(source);

	if (dest)
		free(dest);

	CeRapiUninit();
	return result;
}
