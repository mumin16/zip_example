#include <ziputil.h>

/***************************************************************************/
int main(int, const char **) {
    minizip_opt options;
    int32_t err = 0;

    memset(&options, 0, sizeof(options));

    options.compress_method = MZ_COMPRESS_METHOD_DEFLATE;
    options.compress_level = MZ_COMPRESS_LEVEL_DEFAULT;
    options.include_path = 1;
    //options.append = 1;
    options.overwrite = 1;

    std::vector<std::string> filenames;

    filenames = {"1.txt", "2.txt", "folder"};
    minizip_add("archieve.zip", nullptr, &options, filenames);

    minizip_list("archieve.zip", filenames);
    for (auto &filename: filenames)
        minizip_extract("archieve.zip", nullptr/*nullptr extract all or filename.c_str()*/ , "." /*"./xfolder or "." current android yemiyor"*/, nullptr,
                       &options);

    filenames = {"folder/*"};// "1.txt" or "folder/*" delete folder
    minizip_erase("archieve.zip", nullptr, filenames);

    return err;
}
