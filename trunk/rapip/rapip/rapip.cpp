#include <qfile.h>

#include <kinstance.h>
#include <kmimetype.h>

#include <stdlib.h>
#include <syslog.h>

#include "rapip.h"

#define ANYFILE_BUFFER_SIZE (64*1024)
#define WIDE_BACKSLASH   htole16('\\')

static bool show_hidden_files = true;


kio_rapipProtocol::kio_rapipProtocol(const QCString &pool_socket, const QCString &app_socket)
        : SlaveBase("kio_rapi", pool_socket, app_socket)
{}


kio_rapipProtocol::~kio_rapipProtocol()
{
    closeConnection();
}


void kio_rapipProtocol::openConnection()
{
    HRESULT hr;

    ceOk = true;

    hr = CeRapiInit();
    if (FAILED(hr)) {
        error(KIO::ERR_COULD_NOT_CONNECT, "PDA");
        ceOk = false;
        connected = false;
    } else {
        connected = true;
    }
}


void kio_rapipProtocol::closeConnection()
{
    if (connected)
        CeRapiUninit();
}


WCHAR* kio_rapipProtocol::adjust_remote_path()
{
    WCHAR wide_backslash[2];
    WCHAR path[MAX_PATH];
    WCHAR *returnPath = NULL;

    wide_backslash[0] = WIDE_BACKSLASH;
    wide_backslash[1] = '\0';

    if (ceOk) {
        if (!CeGetSpecialFolderPath(CSIDL_PERSONAL, sizeof(path), path)) {
            ceOk = false;
            returnPath = NULL;
        } else {
            synce::wstr_append(path, wide_backslash, sizeof(path));
            synce_trace_wstr(path);
            returnPath = synce::wstrdup(path);
        }
    }
    return returnPath;
}


bool kio_rapipProtocol::list_matching_files(WCHAR *wide_path)
{
    bool success = false;
    CE_FIND_DATA *find_data = NULL;
    DWORD file_count = 0;
    KIO::UDSEntry udsEntry;
    CE_FIND_DATA *entry = NULL;
    KIO::UDSAtom atom;
    KMimeType::Ptr mt;
    KURL tmpUrl;



    if (ceOk) {
        ceOk = CeFindAllFiles(
                   wide_path,
                   (show_hidden_files ? 0 : FAF_ATTRIB_NO_HIDDEN) |
                   FAF_ATTRIBUTES|FAF_LASTWRITE_TIME|FAF_NAME|FAF_SIZE_LOW|FAF_OID,
                   &file_count, &find_data);

        if (ceOk) {
            for (DWORD i = 0; i < file_count; i++) {
                udsEntry.clear();
                entry = find_data + i;

                atom.m_uds = KIO::UDS_NAME;
                atom.m_str = synce::wstr_to_ascii(entry->cFileName);
                udsEntry.append( atom );

                atom.m_uds = KIO::UDS_SIZE;
                atom.m_long = entry->nFileSizeLow;
                udsEntry.append(atom);

                atom.m_uds = KIO::UDS_ACCESS;
                atom.m_long = S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IWOTH |S_IXUSR | S_IXGRP | S_IXOTH;
                udsEntry.append(atom);

                atom.m_uds = KIO::UDS_MODIFICATION_TIME;
                atom.m_long = synce::filetime_to_unix_time(&entry->ftLastWriteTime);
                udsEntry.append(atom);

                if (entry->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    atom.m_uds = KIO::UDS_FILE_TYPE;
                    atom.m_long = S_IFDIR;
                    udsEntry.append(atom);

                    atom.m_uds = KIO::UDS_MIME_TYPE;
                    atom.m_str = "inode/directory";
                } else {
                    atom.m_uds = KIO::UDS_FILE_TYPE;
                    atom.m_long = S_IFREG;
                    udsEntry.append(atom);

                    tmpUrl.setPath(synce::wstr_to_ascii(entry->cFileName));
                    mt = KMimeType::findByURL(tmpUrl);
                    atom.m_uds = KIO::UDS_MIME_TYPE;
                    atom.m_str=mt->name();
                }
                udsEntry.append(atom);
                listEntry(udsEntry, false);
            }
            listEntry(udsEntry, true);
            success = true;
        }
        CeRapiFreeBuffer(find_data);
    }
    return success;
}


void kio_rapipProtocol::get(const KURL& url)
{
    size_t bytes_read;
    unsigned char buffer[ANYFILE_BUFFER_SIZE];
    QByteArray array;
    KIO::filesize_t processed_size = 0;
    WCHAR* wide_filename;
    QString fName;
    KMimeType::Ptr mt;


    openConnection();

    if (ceOk) {
        mt = KMimeType::findByURL(url);
        mimeType(mt->name());
        fName = QFile::encodeName(url.path());
        if ((wide_filename = synce::wstr_from_ascii(fName.ascii()))) {
            remote = CeCreateFile(wide_filename, GENERIC_READ, 0, NULL,
                                  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
            if (!(INVALID_HANDLE_VALUE == remote)) {
                do {
                    if (ceOk = CeReadFile(remote, buffer, ANYFILE_BUFFER_SIZE, &bytes_read, NULL)) {
                        if (0 == bytes_read) {
                            break;
                        }
                        array.setRawData((char *) buffer, bytes_read);
                        data(array);
                        array.resetRawData((char *) buffer, bytes_read);

                        processed_size += bytes_read;
                        processedSize(processed_size);
                    }
                } while (ceOk);

                if (ceOk) {
                    data(QByteArray());
                    processedSize(processed_size);
                    finished();
                } else {
                    error(KIO::ERR_COULD_NOT_READ, url.path());
                }
                CeCloseHandle(remote);
            } else {
                error(KIO::ERR_CANNOT_OPEN_FOR_READING, url.path());
            }
            synce::wstr_free_string(wide_filename);
        } else {
            error(KIO::ERR_MALFORMED_URL, url.path());
        }
    }
    closeConnection();
}


void kio_rapipProtocol::put(const KURL& url, int /* mode */, bool overwrite, bool /* resume */)
{
    int result;
    WCHAR* wide_filename;
    size_t bytes_written;
    QByteArray buffer;
    KMimeType::Ptr mt;
    QString qPath;


    openConnection();

    if (ceOk) {
        mt = KMimeType::findByURL(url);
        emit mimeType(mt->name());
        qPath = QFile::encodeName(url.path());
        //        qPath.replace('/', "\\");
        if ((wide_filename = synce::wstr_from_ascii(qPath.ascii()))) {
            if (wide_filename)
                for (WCHAR* p = wide_filename; *p; p++)
                    if (*p == '/')
                        *p = '\\';
            if (CeGetFileAttributes(wide_filename) !=  0xFFFFFFFF) {
                if (overwrite) {
                    if (!(ceOk = CeDeleteFile(wide_filename))) {
                        error(KIO::ERR_CANNOT_DELETE, url.path());
                    }
                } else {
                    ceOk = false;
                }
            }
            if (ceOk) {
                remote = CeCreateFile(wide_filename, GENERIC_WRITE, 0, NULL,
                                      CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
                if (!(INVALID_HANDLE_VALUE == remote)) {
                    do {
                        dataReq();
                        result = readData(buffer);
                        if (result > 0) {
                            ceOk = CeWriteFile(remote, (unsigned char *) buffer.data(), buffer.size(), &bytes_written, NULL);
                        }
                    } while (result > 0 && ceOk);

                    if (ceOk) {
                        finished();
                    } else {
                        error(KIO::ERR_COULD_NOT_WRITE, url.path());
                    }
                    CeCloseHandle(remote);
                } else {
                    error(KIO::ERR_CANNOT_OPEN_FOR_WRITING, url.path());
                }
            } else {
                error(KIO::ERR_FILE_ALREADY_EXIST, url.path());
            }
            synce::wstr_free_string(wide_filename);
        } else {
            error(KIO::ERR_MALFORMED_URL, url.path());
        }
    }
    closeConnection();
}


void kio_rapipProtocol::listDir(const KURL& _url)
{
    KURL url(_url);
    QString qPath;
    WCHAR* wide_path;
    char *myDocs;

    openConnection();

    if (ceOk) {
        qPath = QFile::encodeName(url.path());
        if (qPath.isEmpty()) {
            wide_path = adjust_remote_path();
            if (wide_path)
                for (WCHAR *p = wide_path; *p; p++)
                    if (*p == '\\')
                        *p = '/';
            myDocs = synce::wstr_to_ascii(wide_path);
            url.setPath(myDocs);
            synce::wstr_free_string(wide_path);
            synce::wstr_free_string(myDocs);
            redirection(url);
        } else {
            if (qPath.right(1) != "/") {
                qPath = qPath.append('/');
            }
            //            qPath.replace('/', "\\");
            qPath.append('*');
            if ((wide_path = synce::wstr_from_ascii(qPath.ascii()))) {
                if (wide_path)
                    for (WCHAR* p = wide_path; *p; p++)
                        if (*p == '/')
                            *p = '\\';
                if (!list_matching_files(wide_path)) {
                    error(KIO::ERR_CANNOT_ENTER_DIRECTORY, url.path());
                }
                synce::wstr_free_string(wide_path);
            } else {
                error(KIO::ERR_MALFORMED_URL, url.path());
            }
        }
        finished();
    }
    closeConnection();
}


void kio_rapipProtocol::mkdir(const KURL& url, int /* permissions */)
{
    WCHAR *wide_path;
    QString qPath;


    openConnection();

    if (ceOk) {
        qPath = QFile::encodeName(url.path());
        //        qPath.replace('/', "\\");
        if ((wide_path = synce::wstr_from_ascii(qPath.ascii()))) {
            if (wide_path)
                for (WCHAR* p = wide_path; *p; p++)
                    if (*p == '/')
                        *p = '\\';
            if (CeCreateDirectory(wide_path, NULL)) {
                finished();
            } else {
                error(KIO::ERR_DIR_ALREADY_EXIST, url.path());
            }
            synce::wstr_free_string(wide_path);
        } else {
            error(KIO::ERR_MALFORMED_URL, url.path());
        }
    }
    closeConnection();
}


void kio_rapipProtocol::del(const KURL& url, bool isFile)
{
    WCHAR *wide_path;


    openConnection();

    if (ceOk) {
        QString qPath( QFile::encodeName(url.path()));
        //        qPath.replace('/', "\\");
        if ((wide_path = synce::wstr_from_ascii(qPath.ascii()))) {
            if (wide_path)
                for (WCHAR* p = wide_path; *p; p++)
                    if (*p == '/')
                        *p = '\\';
            if (isFile) {
                ceOk = CeDeleteFile(wide_path);
            } else {
                ceOk = CeRemoveDirectory(wide_path);
            }
            if(ceOk) {
                finished();
            } else {
                error(KIO::ERR_CANNOT_DELETE, url.path());
            }
            synce::wstr_free_string(wide_path);
        } else {
            error(KIO::ERR_MALFORMED_URL, url.path());
        }
    }
    closeConnection();
}


void kio_rapipProtocol::stat(const KURL & url)
{
    KIO::UDSEntry udsEntry;
    KIO::UDSAtom atom;
    WCHAR* wide_path;
    DWORD attributes;
    KMimeType::Ptr mt;
    QString qPath;


    openConnection();

    if (ceOk) {
        qPath = QFile::encodeName(url.path());
        //        qPath.replace('/', "\\");
        if ((wide_path = synce::wstr_from_ascii(qPath.ascii()))) {
            if (wide_path)
                for (WCHAR* p = wide_path; *p; p++)
                    if (*p == '/')
                        *p = '\\';
            if ((attributes = CeGetFileAttributes(wide_path)) !=  0xFFFFFFFF) {
                atom.m_uds = KIO::UDS_NAME;
                atom.m_str = url.filename();
                udsEntry.append(atom);

                atom.m_uds = KIO::UDS_SIZE;
                atom.m_long = 1024;
                udsEntry.append(atom);

                atom.m_uds = KIO::UDS_ACCESS;
                atom.m_long = S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IWOTH |S_IXUSR | S_IXGRP | S_IXOTH;
                udsEntry.append(atom);

                if (attributes & FILE_ATTRIBUTE_DIRECTORY) {
                    atom.m_uds = KIO::UDS_FILE_TYPE;
                    atom.m_long = S_IFDIR;
                    udsEntry.append(atom);

                    atom.m_uds = KIO::UDS_MIME_TYPE;
                    atom.m_str="inode/directory";

                    mimeType(atom.m_str);
                } else {
                    atom.m_uds = KIO::UDS_FILE_TYPE;
                    atom.m_long = S_IFREG;
                    udsEntry.append(atom);

                    mt = KMimeType::findByURL(url);
                    atom.m_uds = KIO::UDS_MIME_TYPE;
                    atom.m_str=mt->name();

                    mimeType(atom.m_str);
                }
                udsEntry.append(atom);
                statEntry(udsEntry);

                finished();
            } else {
                error(KIO::ERR_DOES_NOT_EXIST, url.path());
            }
            synce::wstr_free_string(wide_path);
        } else {
            error(KIO::ERR_MALFORMED_URL, url.path());
        }
    }
    closeConnection();
}


void kio_rapipProtocol::mimetype( const KURL& url)
{
    QString qPath;
    WCHAR *wide_path;
    DWORD attributes;
    KMimeType::Ptr mt;


    openConnection();

    if (ceOk) {
        qPath = QFile::encodeName(url.path());
        //        qPath.replace('/', "\\");
        if ((wide_path = synce::wstr_from_ascii(qPath.ascii()))) {
            if (wide_path)
                for (WCHAR* p = wide_path; *p; p++)
                    if (*p == '/')
                        *p = '\\';
            if ((attributes = CeGetFileAttributes(wide_path)) !=  0xFFFFFFFF) {
                if (attributes & FILE_ATTRIBUTE_DIRECTORY) {
                    mimeType("inode/directory");
                } else {
                    mt = KMimeType::findByURL(url);
                    mimeType(mt->name());
                }
                finished();
            } else {
                error(KIO::ERR_DOES_NOT_EXIST, url.path());
            }
            synce::wstr_free_string(wide_path);
        } else {
            error(KIO::ERR_MALFORMED_URL, url.path());
        }
    }
    closeConnection();
}


void kio_rapipProtocol::rename (const KURL& src, const KURL& dst, bool overwrite)
{
    QString sPath;
    QString dPath;
    WCHAR *src_path;
    WCHAR *dst_path;


    openConnection();

    if (ceOk) {
        sPath = QFile::encodeName(src.path());
        //        sPath.replace('/', "\\");
        if ((src_path = synce::wstr_from_ascii(sPath.ascii()))) {
            if (src_path)
                for (WCHAR* p = src_path; *p; p++)
                    if (*p == '/')
                        *p = '\\';
            dPath = QFile::encodeName(dst.path());
            //            dPath.replace('/', "\\");
            if ((dst_path = synce::wstr_from_ascii(dPath.ascii()))) {
                if (dst_path)
                    for (WCHAR* p = dst_path; *p; p++)
                        if (*p == '/')
                            *p = '\\';
                if (CeGetFileAttributes(dst_path) !=  0xFFFFFFFF) {
                    if (overwrite) {
                        if (!(ceOk = CeDeleteFile(dst_path))) {
                            error(KIO::ERR_CANNOT_DELETE, dst.path());
                        }
                    } else {
                        error(KIO::ERR_FILE_ALREADY_EXIST, dPath.ascii());
                        ceOk = false;
                    }
                }
                if (ceOk) {
                    if (CeGetFileAttributes(src_path) !=  0xFFFFFFFF) {
                        if (CeMoveFile(src_path, dst_path)) {
                            finished();
                        } else {
                            error(KIO::ERR_CANNOT_RENAME, dPath);
                        }
                    } else {
                        error(KIO::ERR_DOES_NOT_EXIST, dPath.ascii());
                    }
                }
                synce::wstr_free_string(dst_path);
            } else {
                error(KIO::ERR_MALFORMED_URL, dst.path());
            }
            synce::wstr_free_string(src_path);
        } else {
            error(KIO::ERR_MALFORMED_URL, src.path());
        }
    }
    closeConnection();
}


void kio_rapipProtocol::copy (const KURL& src, const KURL& dst, int /* permissions */, bool overwrite)
{
    QString sPath;
    QString dPath;
    WCHAR *src_path;
    WCHAR *dst_path;


    openConnection();

    if (ceOk) {
        sPath = QFile::encodeName(src.path());
        //        sPath.replace('/', "\\");
        if ((src_path = synce::wstr_from_ascii(sPath.ascii()))) {
            if (src_path)
                for (WCHAR* p = src_path; *p; p++)
                    if (*p == '/')
                        *p = '\\';
            dPath = QFile::encodeName(dst.path());
            //            dPath.replace('/', "\\");
            if ((dst_path = synce::wstr_from_ascii(dPath.ascii()))) {
                if (dst_path)
                    for (WCHAR* p = dst_path; *p; p++)
                        if (*p == '/')
                            *p = '\\';
                if (CeGetFileAttributes(dst_path) !=  0xFFFFFFFF) {
                    if (overwrite) {
                        if (!(ceOk = CeDeleteFile(dst_path))) {
                            error(KIO::ERR_CANNOT_DELETE, dst.path());
                        }
                    } else {
                        error(KIO::ERR_FILE_ALREADY_EXIST, dPath.ascii());
                        ceOk = false;
                    }
                }
                if (ceOk) {
                    if (CeGetFileAttributes(src_path) !=  0xFFFFFFFF) {
                        if (CeCopyFile(src_path, dst_path, true)) {
                            finished();
                        } else {
                            error(KIO::ERR_CANNOT_RENAME, dPath);
                        }
                    } else {
                        error(KIO::ERR_DOES_NOT_EXIST, dPath.ascii());
                    }
                }
                synce::wstr_free_string(dst_path);
            } else {
                error(KIO::ERR_MALFORMED_URL, dst.path());
            }
            synce::wstr_free_string(src_path);
        } else {
            error(KIO::ERR_MALFORMED_URL, src.path());
        }
    }
    closeConnection();
}


extern "C"
{
    int kdemain(int argc, char **argv) {
        KInstance instance( "kio_rapi" );

        if (argc != 4) {
            exit(-1);
        }

        kio_rapipProtocol slave(argv[2], argv[3]);
        slave.dispatchLoop();
        openlog("Rapi", LOG_NOWAIT, LOG_USER);

        return 0;
    }
}
