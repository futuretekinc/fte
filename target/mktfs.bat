mkdir www
mkdir www\css
mkdir www\scripts

copy /Y web_pages\* www\
copy /Y web_pages\css\* www\css\
copy /Y web_pages\scripts\json.js www\scripts\
copy /Y web_pages\scripts\json_parse.js www\scripts\
copy /Y web_pages\scripts\request.js www\scripts\
copy /Y web_pages\scripts\fts.js www\scripts\

..\tools\mktfs.exe www
copy tfs_data.c source\net\http\fte_tfs_data.c
pause
