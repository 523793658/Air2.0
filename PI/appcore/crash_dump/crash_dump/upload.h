#ifndef _Upload_H_
#define _Upload_H_


void _stdcall dump_file(char* fileString);
//在上层拼接文件的字符串再传到C层
void _stdcall set_upload_file(char* fileString);
void _stdcall set_upload_ftp_msg(char* ftpUrl, char* ftpUserName, char* ftpPassword);
void _stdcall set_upload_info(wchar_t* account, wchar_t* userName);
void _stdcall set_upload_server_version(char* serverVersion);
void _stdcall set_upload_dump(PiBool uploadDump);
void _stdcall upload_file(char* fileString, char* filePrefix);
void _stdcall upload_log_file(char* fileString);


void* _stdcall create_upload_task(char* server_url, char* user_name, char* password, PiBool auto_delete);

void _stdcall upload_task_start(void* task);

void _stdcall add_file_to_upload_task(void* task, const char* local_path, const char* romote_path, const char* compressed_name);

void _stdcall free_upload_task(void* task);


#endif