#include "minizip/mz.h"
#include "minizip/mz_os.h"
#include "minizip/mz_strm.h"
#include "minizip/mz_zip.h"
#include "minizip/mz_zip_rw.h"

#include <cstdio>  /* printf */

#include <vector>
#include <string>

/***************************************************************************/

typedef struct minizip_opt_s {
    uint8_t     include_path;
    int16_t     compress_level;
    uint8_t     compress_method;
    uint8_t     overwrite;
    uint8_t     append;
    int64_t     disk_size;
    uint8_t     follow_links;
    uint8_t     store_links;
    uint8_t     zip_cd;
    int32_t     encoding;
    uint8_t     verbose;
    uint8_t     aes;
    const char *cert_path;
    const char *cert_pwd;
} minizip_opt;

/***************************************************************************/



int32_t minizip_list(const char *path,std::vector<std::string>& filenames);

int32_t minizip_add_entry_cb(void *handle, void *userdata, mz_zip_file *file_info);
int32_t minizip_add_progress_cb(void *handle, void *userdata, mz_zip_file *file_info, int64_t position);
int32_t minizip_add(const char *path, const char *password, minizip_opt *options, std::vector<std::string>& args);

int32_t minizip_extract_entry_cb(void *handle, void *userdata, mz_zip_file *file_info, const char *path);
int32_t minizip_extract_progress_cb(void *handle, void *userdata, mz_zip_file *file_info, int64_t position);
int32_t minizip_extract(const char *path, const char *pattern, const char *destination, const char *password, minizip_opt *options);

int32_t minizip_erase(const char *src_path, const char *target_path, std::vector<std::string>& args);

/***************************************************************************/

int32_t minizip_list(const char *path,std::vector<std::string>& filenames) {
    mz_zip_file *file_info = nullptr;
    int32_t err = MZ_OK;
    void *reader = nullptr;


    mz_zip_reader_create(&reader);
    err = mz_zip_reader_open_file(reader, path);
    if (err != MZ_OK) {
        printf("Error %" PRId32 " opening archive %s\n", err, path);
        mz_zip_reader_delete(&reader);
        return err;
    }

    err = mz_zip_reader_goto_first_entry(reader);

    if (err != MZ_OK && err != MZ_END_OF_LIST) {
        printf("Error %" PRId32 " going to first entry in archive\n", err);
        mz_zip_reader_delete(&reader);
        return err;
    }

    /* Enumerate all entries in the archive */
    do {
        err = mz_zip_reader_entry_get_info(reader, &file_info);

        if (err != MZ_OK) {
            printf("Error %" PRId32 " getting entry info in archive\n", err);
            break;
        }


        /* collect file name */
        filenames.emplace_back(file_info->filename);

        err = mz_zip_reader_goto_next_entry(reader);

        if (err != MZ_OK && err != MZ_END_OF_LIST) {
            printf("Error %" PRId32 " going to next entry in archive\n", err);
            break;
        }
    } while (err == MZ_OK);

    mz_zip_reader_delete(&reader);

    if (err == MZ_END_OF_LIST)
        return MZ_OK;

    return err;
}

/***************************************************************************/

int32_t minizip_add_entry_cb(void *handle, void *userdata, mz_zip_file *file_info) {
    MZ_UNUSED(handle);
    MZ_UNUSED(userdata);

    /* Print the current file we are trying to compress */
    printf("Adding %s\n", file_info->filename);
    return MZ_OK;
}

int32_t minizip_add_progress_cb(void *handle, void *userdata, mz_zip_file *file_info, int64_t position) {
    auto *options = (minizip_opt *)userdata;
    double progress = 0;
    uint8_t raw = 0;

    MZ_UNUSED(userdata);

    mz_zip_writer_get_raw(handle, &raw);

    if (raw && file_info->compressed_size > 0)
        progress = ((double)position / file_info->compressed_size) * 100;
    else if (!raw && file_info->uncompressed_size > 0)
        progress = ((double)position / file_info->uncompressed_size) * 100;

    /* Print the progress of the current compress operation */
    if (options->verbose)
        printf("%s - %" PRId64 " / %" PRId64 " (%.02f%%)\n", file_info->filename, position,
            file_info->uncompressed_size, progress);
    return MZ_OK;
}



int32_t minizip_add(const char *path, const char *password, minizip_opt *options, std::vector<std::string>& args) {
    void *writer = nullptr;
    int32_t err = MZ_OK;
    int32_t err_close = MZ_OK;


    printf("Archive %s\n", path);

    /* Create zip writer */
    mz_zip_writer_create(&writer);
    mz_zip_writer_set_password(writer, password);
    mz_zip_writer_set_aes(writer, options->aes);
    mz_zip_writer_set_compress_method(writer, options->compress_method);
    mz_zip_writer_set_compress_level(writer, options->compress_level);
    mz_zip_writer_set_follow_links(writer, options->follow_links);
    mz_zip_writer_set_store_links(writer, options->store_links);
    mz_zip_writer_set_progress_cb(writer, options, minizip_add_progress_cb);
    mz_zip_writer_set_entry_cb(writer, options, minizip_add_entry_cb);
    mz_zip_writer_set_zip_cd(writer, options->zip_cd);
    if (options->cert_path != nullptr)
        mz_zip_writer_set_certificate(writer, options->cert_path, options->cert_pwd);

    err = mz_zip_writer_open_file(writer, path, options->disk_size, options->append);

    if (err == MZ_OK) {
        for (int i = 0; i < args.size(); i += 1) {

            /* Add file system path to archive */
            err = mz_zip_writer_add_path(writer, args[i].c_str(), nullptr, options->include_path, 1);
            if (err != MZ_OK)
                printf("Error %" PRId32 " adding path to archive %s\n", err, args[i].c_str());
        }
    } else {
        printf("Error %" PRId32 " opening archive for writing\n", err);
    }

    err_close = mz_zip_writer_close(writer);
    if (err_close != MZ_OK) {
        printf("Error %" PRId32 " closing archive for writing %s\n", err_close, path);
        err = err_close;
    }

    mz_zip_writer_delete(&writer);
    return err;
}

/***************************************************************************/

int32_t minizip_extract_entry_cb(void *handle, void *userdata, mz_zip_file *file_info, const char *path) {
    MZ_UNUSED(handle);
    MZ_UNUSED(userdata);
    MZ_UNUSED(path);

    /* Print the current entry extracting */
    printf("Extracting %s\n", file_info->filename);
    return MZ_OK;
}

int32_t minizip_extract_progress_cb(void *handle, void *userdata, mz_zip_file *file_info, int64_t position) {
    auto *options = (minizip_opt *)userdata;
    double progress = 0;
    uint8_t raw = 0;

    MZ_UNUSED(userdata);

    mz_zip_reader_get_raw(handle, &raw);

    if (raw && file_info->compressed_size > 0)
        progress = ((double)position / file_info->compressed_size) * 100;
    else if (!raw && file_info->uncompressed_size > 0)
        progress = ((double)position / file_info->uncompressed_size) * 100;

    /* Print the progress of the current extraction */
    if (options->verbose)
        printf("%s - %" PRId64 " / %" PRId64 " (%.02f%%)\n", file_info->filename, position,
            file_info->uncompressed_size, progress);

    return MZ_OK;
}


int32_t minizip_extract(const char *path, const char *pattern, const char *destination, const char *password, minizip_opt *options) {
    void *reader = nullptr;
    int32_t err = MZ_OK;
    int32_t err_close = MZ_OK;


    printf("Archive %s\n", path);

    /* Create zip reader */
    mz_zip_reader_create(&reader);
    mz_zip_reader_set_pattern(reader, pattern, 1);
    mz_zip_reader_set_password(reader, password);
    mz_zip_reader_set_encoding(reader, options->encoding);
    mz_zip_reader_set_entry_cb(reader, options, minizip_extract_entry_cb);
    mz_zip_reader_set_progress_cb(reader, options, minizip_extract_progress_cb);


    err = mz_zip_reader_open_file(reader, path);

    if (err != MZ_OK) {
        printf("Error %" PRId32 " opening archive %s\n", err, path);
    } else {
        /* Save all entries in archive to destination directory */
        err = mz_zip_reader_save_all(reader, destination);

        if (err == MZ_END_OF_LIST) {
            if (pattern != nullptr) {
                printf("Files matching %s not found in archive\n", pattern);
            } else {
                printf("No files in archive\n");
                err = MZ_OK;
            }
        } else if (err != MZ_OK) {
            printf("Error %" PRId32 " saving entries to disk %s\n", err, path);
        }
    }

    err_close = mz_zip_reader_close(reader);
    if (err_close != MZ_OK) {
        printf("Error %" PRId32 " closing archive for reading\n", err_close);
        err = err_close;
    }

    mz_zip_reader_delete(&reader);
    return err;
}

/***************************************************************************/

int32_t minizip_erase(const char *src_path, const char *target_path, std::vector<std::string>& args) {
    mz_zip_file *file_info = nullptr;
    const char *target_path_ptr = target_path;
    void *reader = nullptr;
    void *writer = nullptr;
    int32_t skip = 0;
    int32_t err = MZ_OK;
    uint8_t zip_cd = 0;
    char bak_path[256];
    char tmp_path[256];

    if (target_path == nullptr) {
        /* Construct temporary zip name */
        strncpy(tmp_path, src_path, sizeof(tmp_path) - 1);
        tmp_path[sizeof(tmp_path) - 1] = 0;
        strncat(tmp_path, ".tmp.zip", sizeof(tmp_path) - strlen(tmp_path) - 1);
        target_path_ptr = tmp_path;
    }

    mz_zip_reader_create(&reader);
    mz_zip_writer_create(&writer);

    /* Open original archive we want to erase an entry in */
    err = mz_zip_reader_open_file(reader, src_path);
    if (err != MZ_OK) {
        printf("Error %" PRId32 " opening archive for reading %s\n", err, src_path);
        mz_zip_reader_delete(&reader);
        return err;
    }

    /* Open temporary archive */
    err = mz_zip_writer_open_file(writer, target_path_ptr, 0, 0);
    if (err != MZ_OK) {
        printf("Error %" PRId32 " opening archive for writing %s\n", err, target_path_ptr);
        mz_zip_reader_delete(&reader);
        mz_zip_writer_delete(&writer);
        return err;
    }

    err = mz_zip_reader_goto_first_entry(reader);

    if (err != MZ_OK && err != MZ_END_OF_LIST)
        printf("Error %" PRId32 " going to first entry in archive\n", err);

    while (err == MZ_OK) {
        err = mz_zip_reader_entry_get_info(reader, &file_info);
        if (err != MZ_OK) {
            printf("Error %" PRId32 " getting info from archive\n", err);
            break;
        }

        /* Copy all entries from original archive to temporary archive
           except the ones we don't want */
        for (int i = 0; i < args.size(); i += 1) {
            skip = 0;
            if (mz_path_compare_wc(file_info->filename, args[i].c_str(), 1) == MZ_OK)
                skip = 1;
        }

        if (skip) {
            printf("Skipping %s\n", file_info->filename);
        } else {
            printf("Copying %s\n", file_info->filename);
            err = mz_zip_writer_copy_from_reader(writer, reader);
        }

        if (err != MZ_OK) {
            printf("Error %" PRId32 " copying entry into new zip\n", err);
            break;
        }

        err = mz_zip_reader_goto_next_entry(reader);

        if (err != MZ_OK && err != MZ_END_OF_LIST)
            printf("Error %" PRId32 " going to next entry in archive\n", err);
    }

    mz_zip_reader_get_zip_cd(reader, &zip_cd);
    mz_zip_writer_set_zip_cd(writer, zip_cd);

    mz_zip_reader_close(reader);
    mz_zip_reader_delete(&reader);

    mz_zip_writer_close(writer);
    mz_zip_writer_delete(&writer);

    if (err == MZ_END_OF_LIST) {
        if (target_path == nullptr) {
            /* Swap original archive with temporary archive, backup old archive if possible */
            strncpy(bak_path, src_path, sizeof(bak_path) - 1);
            bak_path[sizeof(bak_path) - 1] = 0;
            strncat(bak_path, ".bak", sizeof(bak_path) - strlen(bak_path) - 1);

            if (mz_os_file_exists(bak_path) == MZ_OK)
                mz_os_unlink(bak_path);

            if (mz_os_rename(src_path, bak_path) != MZ_OK)
                printf("Error backing up archive before replacing %s\n", bak_path);

            if (mz_os_rename(tmp_path, src_path) != MZ_OK)
                printf("Error replacing archive with temp %s\n", tmp_path);
        }

        return MZ_OK;
    }

    return err;
}

/***************************************************************************/

int main(int , const char **) {
    minizip_opt options;
    int32_t err = 0;

    memset(&options, 0, sizeof(options));

    options.compress_method = MZ_COMPRESS_METHOD_DEFLATE;
    options.compress_level = MZ_COMPRESS_LEVEL_DEFAULT;
    options.include_path=1;
    //options.append = 1;
    options.overwrite = 1;


    std::vector<std::string> filenames;

    filenames={"1.txt","2.txt","folder"};
    minizip_add("archieve.zip", nullptr, &options,  filenames);

    minizip_list("archieve.zip",filenames);
    for (auto & filename : filenames)
        minizip_extract("archieve.zip", filename.c_str()/*nullptr extract all*/ , nullptr /*"./folder"*/, nullptr,
                        &options);

    filenames={"1.txt"};//"folder silmiyor
    minizip_erase("archieve.zip", nullptr, filenames);


    return err;
}

