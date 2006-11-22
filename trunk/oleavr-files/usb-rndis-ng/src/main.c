/*
 * Copyright (C) 2006  Ole André Vadla Ravnås <oleavr@gmail.com>
 *
 * Special thanks to Brian Johnson and those who contributed to the
 * donation of a PXA27x-based PDA so that this driver could be made
 * reality...  You guys rock! :-)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

//#define INSANE_DEBUG 1

#include <stdio.h>
#include <glib.h>
#include <usb.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>

#define RNDIS_MSG_COMPLETION            0x80000000

#define RNDIS_MSG_PACKET                         1
#define RNDIS_MSG_INIT                           2
#define RNDIS_MSG_QUERY                          4
#define RNDIS_MSG_SET                            5

#define RNDIS_STATUS_SUCCESS                     0

#define RNDIS_TIMEOUT_MS                      5000

/* FIXME: don't assume USB 2.0 */
#define HOST_MAX_TRANSFER_SIZE               16384

#define OID_802_3_PERMANENT_ADDRESS     0x01010101
#define OID_802_3_CURRENT_ADDRESS       0x01010102
#define OID_GEN_CURRENT_PACKET_FILTER   0x0001010E

#define NDIS_PACKET_TYPE_DIRECTED       0x00000001
#define NDIS_PACKET_TYPE_MULTICAST      0x00000002
#define NDIS_PACKET_TYPE_ALL_MULTICAST  0x00000004
#define NDIS_PACKET_TYPE_BROADCAST      0x00000008
#define NDIS_PACKET_TYPE_SOURCE_ROUTING 0x00000010
#define NDIS_PACKET_TYPE_PROMISCUOUS    0x00000020
#define NDIS_PACKET_TYPE_SMT            0x00000040
#define NDIS_PACKET_TYPE_ALL_LOCAL      0x00000080
#define NDIS_PACKET_TYPE_GROUP          0x00000100
#define NDIS_PACKET_TYPE_ALL_FUNCTIONAL 0x00000200
#define NDIS_PACKET_TYPE_FUNCTIONAL     0x00000400
#define NDIS_PACKET_TYPE_MAC_FRAME      0x00000800

#define RESPONSE_BUFFER_SIZE                  1025

#define USB_DIR_OUT                              0 /* to device */
#define USB_DIR_IN                            0x80 /* to host */

struct rndis_message {
    guint32 msg_type;
    guint32 msg_len;
} __attribute__ ((packed));

struct rndis_request {
    guint32 msg_type;
    guint32 msg_len;
    guint32 request_id;
} __attribute__ ((packed));

struct rndis_response {
    guint32 msg_type;
    guint32 msg_len;
    guint32 request_id;
    guint32 status;
} __attribute__ ((packed));

struct rndis_init {
    guint32 msg_type;
    guint32 msg_len;
    guint32 request_id;
    guint32 major_version;
    guint32 minor_version;
    guint32 max_transfer_size;
} __attribute__ ((packed));

struct rndis_init_c {
    guint32 msg_type;
    guint32 msg_len;
    guint32 request_id;
    guint32 status;
    guint32 major_version;
    guint32 minor_version;
    guint32 device_flags;
    guint32 medium;
    guint32 max_packets_per_message;
    guint32 max_transfer_size;
    guint32 packet_alignment;
    guint32 af_list_offset;
    guint32 af_list_size;
} __attribute__ ((packed));

struct rndis_query {
    guint32 msg_type;
    guint32 msg_len;
    guint32 request_id;
    guint32 oid;
    guint32 len;
    guint32 offset;
    guint32 handle;
} __attribute__ ((packed));

struct rndis_query_c {
    guint32 msg_type;
    guint32 msg_len;
    guint32 request_id;
    guint32 status;
    guint32 len;
    guint32 offset;
} __attribute__ ((packed));

struct rndis_set {
    guint32 msg_type;
    guint32 msg_len;
    guint32 request_id;
    guint32 oid;
    guint32 len;
    guint32 offset;
    guint32 handle;
} __attribute__ ((packed));

struct rndis_set_c {
    guint32 msg_type;
    guint32 msg_len;
    guint32 request_id;
    guint32 status;
} __attribute__ ((packed));

struct rndis_data {
    guint32 msg_type;
    guint32 msg_len;
    guint32 data_offset;
    guint32 data_len;

    guint32 oob_data_offset;
    guint32 oob_data_len;
    guint32 num_oob;

    guint32 packet_data_offset;
    guint32 packet_data_len;

    guint32 vc_handle;
    guint32 reserved;
} __attribute__ ((packed));

static void
log_data (const gchar *filename,
          const guchar *buf,
          int len)
{
  FILE *f = fopen (filename, "w");
  if (f == NULL)
    return;

  fwrite (buf, len, 1, f);

  fclose (f);
}

static gboolean
rndis_command (usb_dev_handle *h,
               struct rndis_request *req,
               struct rndis_response **resp)
{
  gint len;
  guchar buf[8];
  static guchar response_buf[RESPONSE_BUFFER_SIZE];
  static guint id = 2;
  struct rndis_response *r;

  req->msg_type = GUINT32_TO_LE (req->msg_type);
  req->msg_len = GUINT32_TO_LE (req->msg_len);
  req->request_id = GUINT32_TO_LE (id++);

  len = usb_control_msg (h, USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                         USB_REQ_GET_STATUS, 0, 0, (gchar *) req, req->msg_len,
                         RNDIS_TIMEOUT_MS);
  if (len < 0)
    goto USB_ERROR;
  else if (len != req->msg_len)
    {
      fprintf (stderr, "short write, wrote %d out of %d\n", len,
               req->msg_len);
      goto ERROR;
    }

  len = usb_interrupt_read (h, 0x81, (gchar *) buf, sizeof(buf), RNDIS_TIMEOUT_MS);
  if (len < 0)
    goto USB_ERROR;
  else if (len != sizeof(buf))
    {
      fprintf (stderr, "read %d, expected %d\n", len, sizeof(buf));
      goto ERROR;
    }

  len = usb_control_msg (h, USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                         USB_REQ_CLEAR_FEATURE, 0, 0, (gchar *) response_buf,
                         sizeof(response_buf), RNDIS_TIMEOUT_MS);
  if (len < 0)
    goto USB_ERROR;
  else if (len < sizeof (struct rndis_response))
    goto ERROR;

  *resp = (struct rndis_response *) response_buf;
  r = *resp;

  r->msg_type = GUINT32_FROM_LE (r->msg_type);
  r->msg_len = GUINT32_FROM_LE (r->msg_len);
  r->request_id = GUINT32_FROM_LE (r->request_id);
  r->status = GUINT32_FROM_LE (r->status);

  if (r->msg_type !=
      (GUINT32_FROM_LE (req->msg_type) | RNDIS_MSG_COMPLETION))
    goto ERROR;
  else if (r->request_id != GUINT32_FROM_LE (req->request_id))
    goto ERROR;

  return TRUE;

USB_ERROR:
  fprintf (stderr, usb_strerror ());

ERROR:
  return FALSE;
}

static gboolean
rndis_init (usb_dev_handle *h,
            struct rndis_init_c **response)
{
  struct rndis_init req;
  struct rndis_init_c *resp;

  req.msg_type = RNDIS_MSG_INIT;
  req.msg_len = sizeof (req);

  req.major_version = GUINT32_TO_LE (1);
  req.minor_version = GUINT32_TO_LE (0);
  req.max_transfer_size = GUINT32_TO_LE (HOST_MAX_TRANSFER_SIZE);

  if (!rndis_command (h, (struct rndis_request *) &req,
                      (struct rndis_response **) &resp))
    return FALSE;

  resp->major_version = GUINT32_FROM_LE (resp->major_version);
  resp->minor_version = GUINT32_FROM_LE (resp->minor_version);
  resp->device_flags = GUINT32_FROM_LE (resp->device_flags);
  resp->medium = GUINT32_FROM_LE (resp->medium);
  resp->max_packets_per_message = GUINT32_FROM_LE (
                                    resp->max_packets_per_message);
  resp->max_transfer_size = GUINT32_FROM_LE (resp->max_transfer_size);
  resp->packet_alignment = GUINT32_FROM_LE (resp->packet_alignment);
  resp->af_list_offset = GUINT32_FROM_LE (resp->af_list_offset);
  resp->af_list_size = GUINT32_FROM_LE (resp->af_list_size);

  *response = resp;

  return TRUE;
}

static gboolean
rndis_query (usb_dev_handle *h,
             guint32 oid,
             guchar *result,
             guint *res_len)
{
  struct rndis_query req;
  struct rndis_query_c *resp;

  req.msg_type = RNDIS_MSG_QUERY;
  req.msg_len = sizeof (req) + *res_len;

  req.oid = GUINT32_TO_LE (oid);
  req.len = GUINT32_TO_LE (*res_len);
  req.offset = GUINT32_TO_LE (sizeof (req) - sizeof (struct rndis_message));
  req.handle = GUINT32_TO_LE (0);

  if (!rndis_command (h, (struct rndis_request *) &req,
                      (struct rndis_response **) &resp))
    return FALSE;

  resp->len = GUINT32_FROM_LE (resp->len);
  resp->offset = GUINT32_FROM_LE (resp->offset);

  if (resp->len > *res_len)
    return FALSE;

  if (sizeof (struct rndis_message) + resp->offset + resp->len
      > RESPONSE_BUFFER_SIZE)
    {
      return FALSE;
    }

  memcpy (result,
      (guchar *) resp + sizeof (struct rndis_message) + resp->offset,
      resp->len);
  *res_len = resp->len;

  return TRUE;
}

static gboolean
rndis_set (usb_dev_handle *h,
           guint32 oid,
           guchar *value,
           guint value_len)
{
  gboolean success;
  guchar *buf;
  guint msg_len;
  struct rndis_set *req;
  struct rndis_set_c *resp;

  msg_len = sizeof (struct rndis_set) + value_len;

  buf = g_new (guchar, msg_len);
  req = (struct rndis_set *) buf;

  req->msg_type = RNDIS_MSG_SET;
  req->msg_len = msg_len;

  req->oid = GUINT32_TO_LE (oid);
  req->len = GUINT32_TO_LE (value_len);
  req->offset = GUINT32_TO_LE (sizeof (struct rndis_set)
                               - sizeof (struct rndis_message));
  req->handle = GUINT32_TO_LE (0);

  memcpy (buf + sizeof (struct rndis_set), value, value_len);

  success = rndis_command (h, (struct rndis_request *) req,
                           (struct rndis_response **) &resp);

  g_free (buf);

  return success;
}

typedef struct {
    usb_dev_handle *h;
    gint fd;
    guint host_max_transfer_size;
    guint device_max_transfer_size;
    guint alignment;
} RNDISContext;

static gpointer
recv_thread (gpointer data)
{
  RNDISContext *ctx = data;
  guchar *buf;
  gint len;

  puts ("recv_thread speaking");

  buf = g_new (guchar, ctx->host_max_transfer_size);

  while (TRUE)
    {
      gint remaining;
      guchar *p;

      len = usb_bulk_read (ctx->h, 0x82, (gchar *) buf,
                           ctx->host_max_transfer_size, 1000);
      if (len <= 0)
        {
          /* not a timeout? */
          if (len != -110)
            {
              fprintf (stderr, "recv_thread: usb_bulk_read returned %d: %s\n",
                       len, usb_strerror ());
              goto OUT;
            }
          else
            {
              continue;
            }
        }

#ifdef INSANE_DEBUG
      printf ("recv_thread: usb_bulk_read read %d\n", len);
#endif

      p = buf;
      remaining = len;

      while (remaining)
        {
          struct rndis_data *hdr = (struct rndis_data *) p;
          guchar *eth_buf;

          if (remaining < 8)
            {
              fprintf (stderr, "ignoring short packet with remaining=%d\n",
                       remaining);
              break;
            }

          hdr->msg_type = GUINT32_FROM_LE (hdr->msg_type);
          hdr->msg_len = GUINT32_FROM_LE (hdr->msg_len);

#ifdef INSANE_DEBUG
          printf ("recv_thread: msg_len=%d\n", hdr->msg_len);
#endif

          if (hdr->msg_type != RNDIS_MSG_PACKET)
            {
              fprintf (stderr, "ignoring msg_type=%d\n", hdr->msg_type);
              break;
            }
          else if (hdr->msg_len > remaining)
            {
              fprintf (stderr, "msg_len=%d > remaining=%d\n",
                       hdr->msg_len, remaining);
              break;
            }
          else if (hdr->msg_len == 0)
            {
              fprintf (stderr, "ignoring short message\n");
              break;
            }

          hdr->data_offset = GUINT32_FROM_LE (hdr->data_offset);
          hdr->data_len = GUINT32_FROM_LE (hdr->data_len);

          if (hdr->oob_data_offset ||
              hdr->oob_data_len ||
              hdr->num_oob ||
              hdr->packet_data_offset ||
              hdr->packet_data_len)
            {
              hdr->oob_data_offset = GUINT32_FROM_LE (hdr->oob_data_offset);
              hdr->oob_data_len = GUINT32_FROM_LE (hdr->oob_data_len);
              hdr->num_oob = GUINT32_FROM_LE (hdr->num_oob);
              hdr->packet_data_offset = GUINT32_FROM_LE (hdr->packet_data_offset);
              hdr->packet_data_len = GUINT32_FROM_LE (hdr->packet_data_len);

              printf ("recv_thread: interesting packet:\n");
              printf ("  oob_data_offset=%d\n"
                      "  oob_data_len=%d\n"
                      "  num_oob=%d\n"
                      "  packet_data_offset=%d\n"
                      "  packet_data_len=%d\n",
                      hdr->oob_data_offset,
                      hdr->oob_data_len,
                      hdr->num_oob,
                      hdr->packet_data_offset,
                      hdr->packet_data_len);
            }

          if (sizeof (struct rndis_message) + hdr->data_offset
              + hdr->data_len > hdr->msg_len)
            {
              fprintf (stderr, "ignoring truncated message\n");
              break;
            }

          eth_buf = p + sizeof (struct rndis_message) + hdr->data_offset;

#ifdef INSANE_DEBUG
          printf ("writing ethernet frame with size=%d\n", hdr->data_len);
#endif

          len = write (ctx->fd, eth_buf, hdr->data_len);
          if (len <= 0)
            {
              fprintf (stderr, "recv failed because len = %d: %s\n", len,
                       strerror (errno));
              goto OUT;
            }
          else if (len != hdr->data_len)
            {
              fprintf (stderr, "short write, %d out of %d bytes written\n",
                       len, hdr->data_len);
            }

          p += hdr->msg_len;
          remaining -= hdr->msg_len;
        }
    }

  g_free (buf);

OUT:
  printf ("recv_thread exiting\n");
  return NULL;
}

static gpointer
send_thread (gpointer data)
{
  RNDISContext *ctx = data;
  guchar *buf;
  gint len, result;
#ifdef INSANE_DEBUG
  guint i = 0;
#endif

  puts ("send_thread speaking");

  buf = g_new (guchar, ctx->device_max_transfer_size);

  while (TRUE)
    {
      struct rndis_data *hdr = (struct rndis_data *) buf;
#ifdef INSANE_DEBUG
      gchar str[256];
#endif
      guint msg_len;

      len = read (ctx->fd, buf + sizeof (struct rndis_data),
                  ctx->device_max_transfer_size - sizeof (struct rndis_data));
      if (len <= 0)
        {
          fprintf (stderr, "recv failed because len = %d: %s\n", len,
                   strerror (errno));

          return NULL;
        }

#ifdef INSANE_DEBUG
      printf ("send_thread: relaying %d bytes\n", len);
#endif

      memset (hdr, 0, sizeof (struct rndis_data));

      msg_len = sizeof (struct rndis_data) + len;
      if (msg_len % ctx->alignment != 0)
        {
          guint padding = ctx->alignment - (msg_len % ctx->alignment);

#ifdef INSANE_DEBUG
          printf ("send_thread: message length changed from %d to %d\n",
                  msg_len, msg_len + padding);
#endif

          msg_len += padding;
        }

      hdr->msg_type = GUINT32_TO_LE (RNDIS_MSG_PACKET);
      hdr->msg_len = GUINT32_TO_LE (msg_len);

      hdr->data_offset = GUINT32_TO_LE (sizeof (struct rndis_data) -
                                        sizeof (struct rndis_message));
      hdr->data_len = GUINT32_TO_LE (len);

      result = usb_bulk_write (ctx->h, 0x03, (gchar *) buf, msg_len,
                               RNDIS_TIMEOUT_MS);
      if (result <= 0)
        {
          fprintf (stderr, "send_thread: error occurred: %s\n",
                   usb_strerror ());
          return NULL;
        }

#ifdef INSANE_DEBUG
      sprintf (str, "/tmp/sent_packet_%04d.bin", ++i);
      log_data (str, buf, msg_len);
#endif
    }

  g_free (buf);

  printf ("send_thread exiting\n");
  return NULL;
}

static void
handle_device (struct usb_device *dev)
{
  usb_dev_handle *h;
  struct rndis_init_c *resp;
  guchar mac_addr[6];
  guint mac_addr_len;
  gchar mac_addr_str[20], str[64];
  guint32 pf;
  gint fd = -1, err;
  struct ifreq ifr;
  RNDISContext *ctx1 = NULL, *ctx2 = NULL;

  h = usb_open (dev);
  if (h == NULL)
    goto ERROR;

  if (dev->descriptor.bNumConfigurations > 1)
    {
      printf ("warning: more than one configuration found -- using "
              "configuration 0\n");
    }

  if (dev->config->bNumInterfaces != 2)
    {
      printf ("warning: bNumInterfaces != 2 but %d\n",
              dev->config->bNumInterfaces);
    }

  if (usb_claim_interface (h, 0) != 0)
    goto ERROR;

  if (usb_claim_interface (h, 1) != 0)
    goto ERROR;

  /*
  if (usb_set_configuration (h, 1) != 0)
    goto ERROR;*/

  /*
   * initialize
   */

  puts ("doing rndis_init");
  if (!rndis_init (h, &resp))
    goto OUT;

  printf ("rndis_init succeeded:\n");
  printf ("  major_version = %d\n"
          "  minor_version = %d\n"
          "  device_flags = 0x%08x\n"
          "  medium = %d\n"
          "  max_packets_per_message = %d\n"
          "  max_transfer_size = %d\n"
          "  packet_alignment = %d\n"
          "  af_list_offset = %d\n"
          "  af_list_size = %d\n",
          resp->major_version, resp->minor_version,
          resp->device_flags, resp->medium,
          resp->max_packets_per_message,
          resp->max_transfer_size,
          resp->packet_alignment,
          resp->af_list_offset,
          resp->af_list_size);

  ctx1 = g_new (RNDISContext, 1);
  ctx1->h = h;
  ctx1->host_max_transfer_size = HOST_MAX_TRANSFER_SIZE;
  ctx1->device_max_transfer_size = resp->max_transfer_size;
  ctx1->alignment = 1 << resp->packet_alignment;

  ctx2 = g_new (RNDISContext, 1);

  puts ("doing rndis_query for OID_802_3_PERMANENT_ADDRESS");
  mac_addr_len = 6;
  if (!rndis_query (h, OID_802_3_PERMANENT_ADDRESS, mac_addr, &mac_addr_len))
    goto OUT;

  sprintf (mac_addr_str, "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3],
           mac_addr[4], mac_addr[5]);
  printf ("rndis_query succeeded, got MAC address: %s\n", mac_addr_str);

  puts ("setting packet filter");

  pf = GUINT32_TO_LE (NDIS_PACKET_TYPE_DIRECTED
                      | NDIS_PACKET_TYPE_MULTICAST
                      | NDIS_PACKET_TYPE_BROADCAST);
  //pf = GUINT32_TO_LE (NDIS_PACKET_TYPE_PROMISCUOUS);
  rndis_set (h, OID_GEN_CURRENT_PACKET_FILTER, (guchar *) &pf, sizeof (pf));

  puts ("packet filter set");

  if ((fd = open ("/dev/net/tun", O_RDWR)) < 0)
    goto SYS_ERROR;

  memset (&ifr, 0, sizeof (ifr));
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI;

  if ((err = ioctl (fd, TUNSETIFF, (void *) &ifr)) < 0) {
      goto SYS_ERROR;
  }

  printf ("got device '%s'\n", ifr.ifr_name);

  ctx1->fd = fd;
  memcpy (ctx2, ctx1, sizeof (RNDISContext));

  /* hackish */
  sprintf (str, "ifconfig %s hw ether %s", ifr.ifr_name, mac_addr_str);
  system (str);

  system ("ifconfig tap0 up");

  if (!g_thread_create (recv_thread, ctx1, TRUE, NULL))
    goto SYS_ERROR;

  if (!g_thread_create (send_thread, ctx2, TRUE, NULL))
    goto SYS_ERROR;

  puts ("all good");

  goto OUT;

ERROR:
  fprintf (stderr, "error occurred: %s\n", usb_strerror ());
  goto OUT;

SYS_ERROR:
  fprintf (stderr, "error occurred: %s\n", strerror (errno));
  if (fd > 0)
      close (fd);

  g_free (ctx1);
  g_free (ctx2);

OUT:
  return;
}

gint
main(gint argc, gchar *argv[])
{
  GMainContext *ctx;
  GMainLoop *loop;
  struct usb_bus *busses, *bus;

  g_thread_init (NULL);

  ctx = g_main_context_new ();
  loop = g_main_loop_new (ctx, TRUE);

  usb_init ();

  printf ("scanning for devices\n");

  usb_find_busses ();
  usb_find_devices ();

  busses = usb_get_busses ();
  for (bus = busses; bus; bus = bus->next)
    {
      struct usb_device *dev;

      for (dev = bus->devices; dev; dev = dev->next)
        {
          struct usb_device_descriptor *desc = &dev->descriptor;

          if (desc->bDeviceClass == 0xef &&
              desc->bDeviceSubClass == 0x01 &&
              desc->bDeviceProtocol == 0x01)
            {
              handle_device (dev);
            }
        }
    }

  printf ("done scanning for devices\n");

  g_main_loop_run (loop);

  return 0;
}
