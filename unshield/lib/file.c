/* $Id$ */
#include "unshield_internal.h"
#include "md5/global.h"
#include "md5/md5.h"
#include "cabfile.h"
#include <synce_log.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>    /* for MIN(a,b) */
#include <zlib.h>

static FileDescriptor5* unshield_file_descriptor5(Header* header, int index)/*{{{*/
{
  return (FileDescriptor5*)(
      (uint8_t*)header->file_table + 
      header->file_table[header->cab->directory_count + index]
      );
}/*}}}*/

static FileDescriptor6* unshield_file_descriptor6(Header* header, int index)/*{{{*/
{
  return ((FileDescriptor6*)
    ((uint8_t*)header->file_table + letoh32(header->cab->file_table_offset2)))
    + index;
}/*}}}*/

static bool unshield_get_file_descriptor(Unshield* unshield, int index, FileDescriptor6* file_descriptor)/*{{{*/
{
  bool success = false;
  Header* header = NULL;

  /* XXX: multi-volume support... */
  header = unshield->header_list;

  switch (unshield->major_version)
  {
    case 5:
      {
        FileDescriptor5* fd5 = unshield_file_descriptor5(header, index);

        file_descriptor->status           = FILE_COMPRESSED;
        file_descriptor->name_offset      = letoh32(fd5->name_offset);
        file_descriptor->expanded_size    = letoh32(fd5->expanded_size);
        file_descriptor->compressed_size  = letoh32(fd5->compressed_size);
        file_descriptor->data_offset      = letoh32(fd5->data_offset);
        file_descriptor->volume           = header->index;

        memcpy(file_descriptor->md5, fd5->md5, 16);

        /* XXX: copy more */
      }
      break;

    case 6:
      {
        FileDescriptor6* fd6 = unshield_file_descriptor6(header, index);
        memcpy(file_descriptor, fd6, sizeof(FileDescriptor6));

        /* TODO: convert to little endian */
  
#if 0
        LETOH16(file_descriptor->status);
        LETOH32(file_descriptor->expanded_size);
        LETOH32(file_descriptor->compressed_size);
        LETOH32(file_descriptor->data_offset);
        LETOH16(file_descriptor->volume);
#endif

        if (file_descriptor->previous_copy)
        {
          while (fd6->previous_copy)
            fd6 = unshield_file_descriptor6(header, fd6->previous_copy);

          file_descriptor->data_offset = letoh32(fd6->data_offset);
          file_descriptor->volume      = letoh16(fd6->volume);
        }

#if 0
        if (0 == strcmp(unshield_file_name(unshield, index), "start.htm"))
        {
          synce_warning("File %i (%s) is start.htm. File descriptor offset 0x%08x. Volume %i offset 0x%08x.", 
              index, unshield_file_name(unshield, index),
              (void*)fd6 - (void*)header->data,
              file_descriptor->volume, file_descriptor->data_offset);
        }
#endif

#if 0
        if (!(file_descriptor->status & FILE_COMPRESSED))
        {
          synce_warning("File %i (%s) is not compressed. File descriptor offset 0x%08x. Volume %i offset 0x%08x.", 
              index, unshield_file_name(unshield, index),
              (void*)fd6 - (void*)header->data,
              file_descriptor->volume, file_descriptor->data_offset);
        }
#endif

        if (file_descriptor->status & FILE_ENCRYPTED)
        {
          synce_warning("File %i (%s) is encrypted", 
              index, unshield_file_name(unshield, index));
        }
        
#if 0 
        if (status & FILE_SPLIT)
        {
          synce_trace("Split file in volume %i! Index = %i, Name = '%s', file descriptor offset: 0x%08x", 
              volume, index, unshield_file_name(unshield, index), (uint8_t*)fd - header->data);
        }
#endif
      }
      break;

    default:
      synce_error("Unknown major version: %i", unshield->major_version);
      goto exit;
  }

  success = true;

exit:
  return success;
}/*}}}*/

int unshield_file_count (Unshield* unshield)/*{{{*/
{
  if (unshield)
  {
    /* XXX: multi-volume support... */
    Header* header = unshield->header_list;

    return header->cab->file_count;
  }
  else
    return -1;
}/*}}}*/

const char* unshield_file_name (Unshield* unshield, int index)/*{{{*/
{
  if (unshield)
  {
    /* XXX: multi-volume support... */
    Header* header = unshield->header_list;

    switch (unshield->major_version)
    {
      case 5:
        {
          FileDescriptor5* fd = unshield_file_descriptor5(header, index);
          if (fd->name_offset)
            return (const char*)((uint8_t*)header->file_table + letoh32(fd->name_offset));
          else
            return NULL;
        }
        break;

      case 6:
        {
          FileDescriptor6* fd = unshield_file_descriptor6(header, index);
          if (fd->name_offset)
            return (const char*)((uint8_t*)header->file_table + letoh32(fd->name_offset));
          else
            return NULL;
        }
        break;

      default:
        synce_error("Unknown major version: %i", unshield->major_version);
        return NULL;
    }

  }
  else
    return NULL;
}/*}}}*/

bool unshield_file_is_valid(Unshield* unshield, int index)
{
  bool is_valid = false;
  FileDescriptor6 fd;

  if (index < 0 || index >= unshield_file_count(unshield))
    goto exit;

  if (!unshield_get_file_descriptor(unshield, index, &fd))
    goto exit;

  if (fd.status & FILE_INVALID)
    goto exit;

  if (!fd.name_offset)
    goto exit;

  if (!fd.data_offset)
    goto exit;

  is_valid = true;
  
exit:
  return is_valid;
}


static int unshield_uncompress (Byte *dest, uLong *destLen, Byte *source, uLong sourceLen)/*{{{*/
{
    z_stream stream;
    int err;

    stream.next_in = source;
    stream.avail_in = (uInt)sourceLen;

    stream.next_out = dest;
    stream.avail_out = (uInt)*destLen;

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;

    /* make second parameter negative to disable checksum verification */
    err = inflateInit2(&stream, -MAX_WBITS);
    if (err != Z_OK) return err;

    err = inflate(&stream, Z_FINISH);
    if (err != Z_STREAM_END) {
        inflateEnd(&stream);
        return err;
    }
    *destLen = stream.total_out;

    err = inflateEnd(&stream);
    return err;
}/*}}}*/

typedef struct 
{
  Unshield*         unshield;
  int               index;
  FileDescriptor6*  file_descriptor;
  int               volume;
  FILE*             volume_file;
  VolumeHeader6     volume_header;
  unsigned          volume_bytes_left;
} UnshieldReader;

static bool unshield_reader_open_volume(UnshieldReader* reader, int volume)/*{{{*/
{
  bool success = false;
  unsigned data_offset = 0;
  unsigned volume_bytes_left_compressed;
  unsigned volume_bytes_left_expanded;
  CommonHeader common_header;
  
  FCLOSE(reader->volume_file);

  reader->volume_file = unshield_fopen_for_reading(reader->unshield, volume, CABINET_SUFFIX);
  if (!reader->volume_file)
  {
    synce_error("Failed to open input cabinet file %i", volume);
    goto exit;
  }

  if (sizeof(CommonHeader) != 
      fread(&common_header, 1, sizeof(CommonHeader), reader->volume_file))
    goto exit;

  if (CAB_SIGNATURE != letoh32(common_header.signature))
    goto exit;

  switch (reader->unshield->major_version)
  {
    case 5:
      {
        /* read version 5 header and convert to version 6 header */
        VolumeHeader5 five_header;
        if (sizeof(VolumeHeader5) != 
            fread(&five_header, 1, sizeof(VolumeHeader5), reader->volume_file))
          goto exit;

        memset(&reader->volume_header, 0, sizeof(VolumeHeader6));

        reader->volume_header.data_offset                = letoh32(five_header.data_offset);
        reader->volume_header.first_file_index           = letoh32(five_header.first_file_index);
        reader->volume_header.last_file_index            = letoh32(five_header.last_file_index);
        reader->volume_header.first_file_offset          = letoh32(five_header.first_file_offset);
        reader->volume_header.first_file_size_expanded   = letoh32(five_header.first_file_size_expanded);
        reader->volume_header.first_file_size_compressed = letoh32(five_header.first_file_size_compressed);
        reader->volume_header.last_file_offset           = letoh32(five_header.last_file_offset);
        reader->volume_header.last_file_size_expanded    = letoh32(five_header.last_file_size_expanded);
        reader->volume_header.last_file_size_compressed  = letoh32(five_header.last_file_size_compressed);

      }
      break;

    case 6:
      if (sizeof(VolumeHeader6) != 
          fread(&reader->volume_header, 1, sizeof(VolumeHeader6), reader->volume_file))

        /* TODO: convert to little endian */

        goto exit;
      break;

    default:
      goto exit;
  }

  if (reader->file_descriptor->status & FILE_SPLIT)  /* XXX: handle IS5 too */
  {   
#if 0
    synce_trace("Total bytes left = 0x08%x, previous data offset = 0x08%x",
        total_bytes_left, data_offset); 
#endif

    if (reader->index == reader->volume_header.last_file_index)
    {
      /* can be first file too... */
#if 0
      synce_trace("Index %i is last file in cabinet file %i",
          reader->index, volume);
#endif

      data_offset                   = reader->volume_header.last_file_offset;
      volume_bytes_left_expanded    = reader->volume_header.last_file_size_expanded;
      volume_bytes_left_compressed  = reader->volume_header.last_file_size_compressed;
    }
    else if (reader->index == reader->volume_header.first_file_index)
    {
#if 0
      synce_trace("Index %i is first file in cabinet file %i",
          reader->index, volume);
#endif

      data_offset                   = reader->volume_header.first_file_offset;
      volume_bytes_left_expanded    = reader->volume_header.first_file_size_expanded;
      volume_bytes_left_compressed  = reader->volume_header.first_file_size_compressed;
    }
    else
      abort();

#if 0
    synce_trace("Will read 0x%08x bytes from offset 0x%08x",
        volume_bytes_left_compressed, data_offset);
#endif
  }
  else
  {
    data_offset                  = reader->file_descriptor->data_offset;
    volume_bytes_left_expanded   = reader->file_descriptor->expanded_size;
    volume_bytes_left_compressed = reader->file_descriptor->compressed_size;
  }

  if (reader->file_descriptor->status & FILE_COMPRESSED)
    reader->volume_bytes_left = volume_bytes_left_compressed;
  else
    reader->volume_bytes_left = volume_bytes_left_expanded;

  fseek(reader->volume_file, data_offset, SEEK_SET);

  reader->volume = volume;
  success = true;

exit:
  return success;
}/*}}}*/

static bool unshield_reader_read(UnshieldReader* reader, void* buffer, size_t size)/*{{{*/
{
  bool success = false;

  for (;;)
  {
    /* 
       Read as much as possible from this volume
     */
    size_t bytes_to_read = MIN(size, reader->volume_bytes_left);

    if (bytes_to_read != fread(buffer, 1, bytes_to_read, reader->volume_file))
    {
      synce_error("Failed to read 0x%08x bytes of file %i (%s) from volume %i. Current offset = 0x%08x",
          bytes_to_read, reader->index, 
          unshield_file_name(reader->unshield, reader->index), reader->volume,
          ftell(reader->volume_file));
      goto exit;
    }

    size -= bytes_to_read;
    reader->volume_bytes_left -= bytes_to_read;

    if (!size)
      break;

    buffer += bytes_to_read;

    /*
       Open next volume
     */

    if (!unshield_reader_open_volume(reader, reader->volume + 1))
    {
      synce_error("Failed to open volume %i",
          bytes_to_read, reader->volume);
      goto exit;
    }
  }

  success = true;

exit:
  return success;
}/*}}}*/

static UnshieldReader* unshield_reader_create(/*{{{*/
    Unshield* unshield, 
    int index,
    FileDescriptor6* file_descriptor)
{
  bool success = false;
  
  UnshieldReader* reader = calloc(1, sizeof(UnshieldReader));
  if (!reader)
    return NULL;

  reader->unshield          = unshield;
  reader->index             = index;
  reader->file_descriptor   = file_descriptor;

  if (!unshield_reader_open_volume(reader, file_descriptor->volume))
  {
    synce_error("Failed to open volume %i",
        file_descriptor->volume);
    goto exit;
  }

  success = true;

exit:
  if (success)
    return reader;

  FREE(reader);
  return NULL;
}/*}}}*/

static void unshield_reader_destroy(UnshieldReader* reader)/*{{{*/
{
  if (reader)
  {
    FCLOSE(reader->volume_file);
    free(reader);
  }
}/*}}}*/

#define BUFFER_SIZE (64*1024+1)

bool unshield_file_save (Unshield* unshield, int index, const char* filename)/*{{{*/
{
  bool success = false;
  FILE* output = NULL;
  unsigned char* input_buffer   = (unsigned char*)malloc(BUFFER_SIZE);
  unsigned char* output_buffer  = (unsigned char*)malloc(BUFFER_SIZE);
  int bytes_left;
  uLong total_written = 0;
  UnshieldReader* reader = NULL;
  FileDescriptor6 file_descriptor;
	MD5_CTX md5;

	MD5Init(&md5);

  if (!unshield || !filename)
    goto exit;

  if (!unshield_get_file_descriptor(unshield, index, &file_descriptor))
  {
    synce_error("Failed to get file descriptor for file %i", index);
    goto exit;
  }

  if ((file_descriptor.status & FILE_INVALID) || 0 == file_descriptor.data_offset)
  {
    /* invalid file */
    goto exit;
  }
  
  reader = unshield_reader_create(unshield, index, &file_descriptor);
  if (!reader)
  {
    synce_error("Failed to create data reader for file %i", index);
    goto exit;
  }

  output = fopen(filename, "w");
  if (!output)
  {
    synce_error("Failed to open output file '%s'", filename);
    goto exit;
  }

  if (file_descriptor.status & FILE_COMPRESSED)
    bytes_left = file_descriptor.compressed_size;
  else
    bytes_left = file_descriptor.expanded_size;

  while (bytes_left > 0)
  {
    uLong bytes_to_write = BUFFER_SIZE;
    int result;

    if (file_descriptor.status & FILE_COMPRESSED)
    {
      uint16_t bytes_to_read = 0;

      if (!unshield_reader_read(reader, &bytes_to_read, sizeof(bytes_to_read)))
      {
#if 0
        synce_error("Failed to read %i bytes of file %i (%s) from input cabinet file %i", 
            sizeof(bytes_to_read), index, unshield_file_name(unshield, index), volume);
#endif
        goto exit;
      }

      bytes_to_read = letoh16(bytes_to_read);
      if (!unshield_reader_read(reader, input_buffer, bytes_to_read))
      {
#if 0
        synce_error("Failed to read %i bytes of file %i (%s) from input cabinet file %i", 
            bytes_to_read, index, unshield_file_name(unshield, index), volume);
#endif
        goto exit;
      }

      /* add a null byte to make inflate happy */
      input_buffer[bytes_to_read] = 0;
      result = unshield_uncompress(output_buffer, &bytes_to_write, input_buffer, bytes_to_read+1);

      if (Z_OK != result)
      {
#if 0
        synce_error("Decompression failed with code %i. bytes_to_read=%i, volume_bytes_left=%i", 
            result, bytes_to_read, volume_bytes_left);
#endif
        /*      abort();*/
        goto exit;
      }

      bytes_left -= 2;
      bytes_left -= bytes_to_read;
    }
    else
    {
      bytes_to_write = MIN(bytes_left, BUFFER_SIZE);

      if (!unshield_reader_read(reader, input_buffer, bytes_to_write))
      {
#if 0
        synce_error("Failed to read %i bytes from input cabinet file %i", 
            bytes_to_write, volume);
#endif
        goto exit;
      }

      bytes_left -= bytes_to_write;
    }

    MD5Update(&md5, output_buffer, bytes_to_write);

    if (bytes_to_write != fwrite(output_buffer, 1, bytes_to_write, output))
    {
      synce_error("Failed to write %i bytes to file '%s'", bytes_to_write, filename);
      goto exit;
    }

    total_written += bytes_to_write;
  }

  if (file_descriptor.expanded_size != total_written)
  {
    synce_error("Expanded size expected to be %i, but was %i", file_descriptor.expanded_size, total_written);
    goto exit;
  }

  {
    unsigned char md5result[16];
    MD5Final(md5result, &md5);

    if (0 != memcmp(md5result, file_descriptor.md5, 16))
    {
      synce_error("MD5 checksum failure for file %i (%s)", 
          index, unshield_file_name(unshield, index));
      goto exit;
    }
  }

  success = true;
  
exit:
  unshield_reader_destroy(reader);
  FCLOSE(output);
  FREE(input_buffer);
  FREE(output_buffer);
  return success;
}/*}}}*/

int unshield_file_directory(Unshield* unshield, int index)/*{{{*/
{
  if (unshield)
  {
    /* XXX: multi-volume support... */
    Header* header = unshield->header_list;

    switch (unshield->major_version)
    {
      case 5:
        {
          FileDescriptor5* fd = unshield_file_descriptor5(header, index);
          return fd->directory_index;
        }
        break;

      case 6:
        {
          FileDescriptor6* fd = unshield_file_descriptor6(header, index);
          return fd->directory_index;
        }
        break;

      default:
        synce_error("Unknown major version: %i", unshield->major_version);
        return -1;
    }

  }
  else
    return -1;
}/*}}}*/

