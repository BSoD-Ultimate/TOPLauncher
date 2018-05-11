/* 
 * SJEJhhUtil - encode/decode SJE.JHH files locates in 
 * Tetris Online Poland(TOP) client.
 *
 * Code here is inspired by Wojtek's tool used to pack/unpack "SJE.JHH" files
 * locates in Tetris Online Poland client.
 * 
 * The implementation is partly based on disassembling Wojtek's work of 
 * "Encrypt.exe" and "SjeJhhTool.exe"
 * 
 * Thanks for Wojtek's work help.
*/
#pragma once

#include <stdint.h>

#ifdef SJEJHHUTIL_BUILD_DLL
#ifdef SJEJHHUTIL_EXPORTS
#define SJEJHHUTIL_API __declspec(dllexport)
#else
#define SJEJHHUTIL_API __declspec(dllimport)
#endif
#else
#define SJEJHHUTIL_API extern
#endif 

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    /*
     * sjejhh_encrypt_data
     * 
     * Encrypt plain data in memory into the encrypted TOP client-compatible format.
     * 
     * Parameters: 
     * 
     * data:        plain data needs to be encrypted.
     * inputLength: plain data size.
     * out:         pointer to a part of memory which receives the encrypted data.
     * outLength:   output memory buffer length.
     * keyData:     the binary-formatted key used in encryption.
     * keyLength:   length of encryption key data. 
     *
     * When calling this function. An encryption key in binary-format is required.
     * The parameter "keyLength" should not be zero. Otherwise, the encryption 
     * will fail.
     * When encryption succeeded, the function returns 0.
     * When encryption failed, the return value is -1.
     * When parameter "outLength"'s value is 0, or the encrypted data requires more
     * space than parameter "outLength" assigned, the return value will be the minimum 
     * size of memory space required to hold the encrypted data, which helps user to 
     * allocate enough memory space to store the encrypted result.
    */
    SJEJHHUTIL_API int sjejhh_encrypt_data(
        const char* data,
        size_t inputLength,
        char* out,
        size_t outLength,
        const char* keyData,
        uint32_t keyLength
    );

    /*
     * sjejhh_encrypt_file
     * 
     * Encrypt a plain data file into the TOP-Client-accepted encrypted format. 
     * The encrypted file will be saved in the path points to "output_file_path".
     * 
     * Parameters:
     *
     * input_filePath:  path points to the source file.
     * output_filePath: the path where the encrypted file will be saved.
     * keyData:         the binary-formatted key used in encryption.
     * keyLength:       length of encryption key data.
     * 
     * An encryption key in binary-format is required. The parameter "keyLength"
     * should not be zero. Otherwise, the encryption will fail.
     * When encryption succeeded, the function returns 1.
     * When encryption failed, the return value is 0.
     * 
    */
    SJEJHHUTIL_API int sjejhh_encrypt_file(
        const wchar_t* input_filePath,
        const wchar_t* output_filePath,
        const char* keyData,
        uint32_t keyLength
    );

    /*
     * Sugar of encryption methods. :D
     * 
     * CAUTION: Functionality of the following two functions may be altered later.
     * 
     * The way of encryption used in the TOP client is RC4, which means when a piece of
     * encrypted data is encrypted again(or a piece of plain data encrypted twice),
     * the result is still plain data. In short:
     * 
     * Encrypt(Encrypt(plain data)) = plain data.
     * 
     * See more information about RC4 in wikipedia:
     * https://en.wikipedia.org/wiki/RC4
     *
     * So decryption functions here is just an alias of encryption functions. Just for 
     * distinguishing when they are used together with "encrypt" methods. :P
    */

    /*
     * alias of function sjejhh_encrypt_data.
    */
    SJEJHHUTIL_API int sjejhh_decrypt_data(
        const char* data,
        size_t inputLength,
        char* out,
        size_t outLength,
        const char* keyData,
        uint32_t keyLength
    );

    /*
     * alias of function sjejhh_encrypt_file.
    */
    SJEJHHUTIL_API int sjejhh_decrypt_file(
        const wchar_t* input_filePath,
        const wchar_t* output_filePath,
        const char* keyData, 
        uint32_t keyLength
    );


    /* *********************************************************************************
     * APIs used for unpacking "SJE.JHH" archives.
     *
     * ********************************************************************************
    */

    /*
     * Error codes
    */
    typedef enum _sjejhh_unpack_error_code
    {
        SJEJHH_UNPACK_OK = 0,
        SJEJHH_UNPACK_END_OF_LIST_OF_FILE,
        SJEJHH_UNPACK_ERRNO,
        SJEJHH_UNPACK_EOF,
        SJEJHH_UNPACK_INVALIDARG,
        SJEJHH_UNPACK_INTERNALERROR,

    }sjejhh_unpack_error_code;

    typedef struct sjejhh_unpack_context sjejhh_unpack_context;

    /*
     * global information of a "SJE.JHH" archive
    */
    typedef struct _sjejhh_unpack_global_info
    {
        // "SJE.JHH" file path
        const wchar_t* archiveFilePath;
        size_t archiveFilePathLength;

        // internal folder identifier
        const char* internalFolderName;
        size_t internalFolderNameLength;

        size_t archiveFileSize;   // file size
        size_t fileCount;         // total file count
        size_t currentFileIndex;  // current file index
    }sjejhh_unpack_global_info;

    /*
     * information of a file in "SJE.JHH" archive
    */
    typedef struct _sjejhh_unpack_file_info
    {
        size_t currentIndex;

        const wchar_t* filename;
        size_t filenameLength;

        uint32_t fileOffset;
        uint32_t fileLength;
        int isEncrypted;
    }sjejhh_unpack_file_info;

    /*
     * sjejhh_unpack_open
     *
     * Open a "SJE.JHH" archive file.
     *
     * Parameters:
     *
     * filePath: Path string where an "SJE.JHH" archive file locates
     *
     * Open a opaque handle which can be used for unpacking files in an "SJE.JHH" archive file.
     * When the function fails, the return value is NULL.
    */
    SJEJHHUTIL_API sjejhh_unpack_context* sjejhh_unpack_open(const wchar_t* filePath);

    /*
     * sjejhh_unpack_get_global_info
     *
     * Get global information of an "SJE.JHH" archive file which opened by a 
     * sjejhh_unpack_context handle.
     *
     * Parameters:
     *
     * pArchive: A handle used to retrieve global information from the archive file.
     * pGlobalInfo: pointer to a structure which receives global information from the handle.
     *
    */
    SJEJHHUTIL_API int sjejhh_unpack_get_global_info(
        sjejhh_unpack_context* pArchive,
        sjejhh_unpack_global_info* pGlobalInfo
    );

    /*
     * sjejhh_unpack_get_current_file_info
     *
     * Get file information about the subfile which the internal file cursor in the 
     * sjejhh_unpack_context handle currently points to.
     *
     * Parameters:
     *
     * pArchive: A handle used to retrieve the current subfile's information.
     * pCurrentFileInfo: pointer to a structure which receives current subfile's information.
     *
     * When the file cursor reaches the end of the internal file list in the handle,
     * the function returns SJEJHH_UNPACK_END_OF_LIST_OF_FILE.
    */
    SJEJHHUTIL_API int sjejhh_unpack_get_current_file_info(
        sjejhh_unpack_context* pArchive,
        sjejhh_unpack_file_info* pCurrentFileInfo
    );

    /*
     * sjejhh_unpack_read_current_file
     *
     * Read contents of the subfile which the internal file cursor in the 
     * sjejhh_unpack_context handle currently points to.
     *
     * Parameters:
     *
     * pArchive:       The handle which contents of subfiles needed to be read.
     * readBuf:        Pointer to the memory where file content read from the archive will be stored.
     * bufLength:      Size of the memory which stores the received content.
     * bytesRead:      Receives how many bytes are actually read from the archive file.
     * bytesRemaining: Receives how many bytes remain before the whole subfile content is read.
     *
     * By using this function, subfile contents are read in stream-format, which means when calling
     * this method repeatingly, the function returns the contents after the previous call has read 
     * until whole contents of the subfile have been read (In other words, reading operation reaches EOF).
     * 
     * When reading the contents failed, the function returns SJEJHH_UNPACK_ERRNO.
     * When all contents have been read, the function returns SJEJHH_UNPACK_EOF.
     * When file cursor locates at the end of the file list, the function
     * returns SJEJHH_UNPACK_END_OF_LIST_OF_FILE.
    */
    SJEJHHUTIL_API int sjejhh_unpack_read_current_file(
        sjejhh_unpack_context* pArchive,
        char* readBuf,
        size_t bufLength, 
        size_t* bytesRead,
        size_t* bytesRemaining
    );

    /*
     * sjejhh_unpack_decrypt_read_data
     *
     * If contents read from the subfile using function sjejhh_unpack_read_current_file is encrypted, 
     * use this function to decrypt the data then receive the original content of the subfile.
     *
     * Parameters:
     *
     * pArchive:  A handle used to retrieve the current subfile's information.
     * inData:    Data which needs to be decrypted.
     * inLength:  Length of the encrypted data.
     * outBuf:    Buffer which Receives the decrypted result data.
     * outLength: Length of the buffer which is used to receive the decrypted result.
     *
     * When decryption succeeded, the function returns 0.
     * When decryption failed, the return value is -1.
     * When parameter "outLength"'s value is 0, or the encrypted data requires more
     * space than parameter "outLength" assigned, the return value will be the minimum 
     * size of memory space required to hold the encrypted data, which helps user to 
     * allocate enough memory space to store the encrypted result.
     * 
     * CAUTION: Tetris Online Poland client is using Microsoft's cryptography API in a wrong way.
     * You may need to call the sjejhh_unpack_reset_decrypt_context function to reset
     * the decryption context manually to get the data accepted by the client.
     * 
    */
    SJEJHHUTIL_API int sjejhh_unpack_decrypt_read_data(
        sjejhh_unpack_context* pArchive,
        const char* inData,
        size_t inLength,
        char* outBuf,
        size_t outLength
    );

    /*
     * sjejhh_unpack_reset_decrypt_context
     *
     * Reset the decryption context used by the unpacking handle
     *
     * Parameters:
     *
     * pArchive: The handle whose decryption context is going to be reset.
     *
    */
    SJEJHHUTIL_API void sjejhh_unpack_reset_decrypt_context(sjejhh_unpack_context* pArchive);

    /*
     * sjejhh_unpack_seek_to_begin
     *
     * Reset the file cursor to the beginning of the file list in the handle.
     *
     * Parameters:
     *
     * pArchive: The handle which is going to reset the file pointer.
     *
     * When the "SJE.JHH" file the handle opened contains no file, 
     * the function returns SJEJHH_UNPACK_END_OF_LIST_OF_FILE.
    */
    SJEJHHUTIL_API int sjejhh_unpack_seek_to_begin(sjejhh_unpack_context* pArchive);

    /*
     * sjejhh_unpack_reset_file_pointer
     *
     * Reset the internal file pointer which opens the "SJE.JHH" file to the location where
     * the beginning of contents of the file that the file cursor currently points to.
     *
     * Parameters:
     *
     * pArchive: The handle which is going to reset the file pointer.
     *
     * When the file cursor reaches the end of the internal file list in the handle,
     * the function returns SJEJHH_UNPACK_END_OF_LIST_OF_FILE.
    */
    SJEJHHUTIL_API int sjejhh_unpack_reset_file_pointer(sjejhh_unpack_context* pArchive);

    /*
     * sjejhh_unpack_goto_next_file
     *
     * Move file cursor to the next file element in the internal file list in the handle.
     *
     * Parameters:
     *
     * pArchive: The handle which needs to move the file cursor.
     *
     * When the file cursor reaches the end of the internal file list in the handle,
     * the function returns SJEJHH_UNPACK_END_OF_LIST_OF_FILE.
    */
    SJEJHHUTIL_API int sjejhh_unpack_goto_next_file(sjejhh_unpack_context* pArchive);

    /*
     * sjejhh_unpack_close
     *
     * Close the "SJE.JHH" file handle opened by sjejhh_unpack_open.
     *
     * Parameters:
     *
     * pArchive: The handle which is going to be closed.
     *
    */
    SJEJHHUTIL_API int sjejhh_unpack_close(sjejhh_unpack_context* pArchive);




    /*
    * functions used to pack a "SJE.JHH" archive
    */
    typedef struct sjejhh_pack_context sjejhh_pack_context;

    typedef enum _sjejhh_pack_error_code
    {
        SJEJHH_PACK_OK = 0,
        SJEJHH_PACK_ERRNO,
        SJEJHH_PACK_NO_SUCH_FILE,

    }sjejhh_pack_error_code;

    typedef struct _sjejhh_pack_global_info
    {
        const char* internalFolderName;
        size_t internalFolderNameLength;

        const wchar_t* fileSavePath;
        size_t savePathLength;

        size_t fileCount;

    }sjejhh_pack_global_info;

    typedef enum _sjejhh_pack_file_type
    {
        SJEJHH_PACK_FILETYPE_UNKNOWN,
        SJEJHH_PACK_FILETYPE_FILEPATH, 
        SJEJHH_PACK_FILETYPE_MEMORYDATA,
    }sjejhh_pack_file_type;

    typedef struct _sjejhh_pack_file_info
    {
        sjejhh_pack_file_type fileType;

        const wchar_t* filename;
        size_t filenameLength;

        const void* fileData;

    }sjejhh_pack_file_info;

    typedef int(*sjejhh_pack_enum_file_callback)(sjejhh_pack_context*, sjejhh_pack_file_info*, void*);

    SJEJHHUTIL_API sjejhh_pack_context* sjejhh_pack_create_file(
        const char* internalFolderName,
        const wchar_t* filePath
    );

    SJEJHHUTIL_API int sjejhh_pack_get_global_info(
        sjejhh_pack_context* pArchive,
        sjejhh_pack_global_info* globalInfo
    );

    SJEJHHUTIL_API int sjejhh_pack_add_file(
        sjejhh_pack_context* pArchive,
        const wchar_t* filePath
    );

    SJEJHHUTIL_API int sjejhh_pack_add_memory_data(
        sjejhh_pack_context* pArchive,
        const char* inputBuf,
        uint32_t inputBufLen,
        const wchar_t* filename,
        int doCopyData
    );

    SJEJHHUTIL_API int sjejhh_pack_remove_file(
        sjejhh_pack_context* pArchive,
        const wchar_t* filenameOrPath
    );

    SJEJHHUTIL_API int sjejhh_pack_enum_files(
        sjejhh_pack_context* pArchive,
        sjejhh_pack_enum_file_callback enumCallback,
        void* userdata
    );

    SJEJHHUTIL_API int sjejhh_pack_do_pack(sjejhh_pack_context* pArchive);

    SJEJHHUTIL_API int sjejhh_pack_close(sjejhh_pack_context* pArchive);



#ifdef __cplusplus
}
#endif // __cplusplus
