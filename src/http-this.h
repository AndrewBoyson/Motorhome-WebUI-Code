
extern int HttpThisNameValue(unsigned rid, char* name, char* value );
extern int HttpThisInclude  (char* name, char* format); // Returns 0 if handled, 1 if not handled

extern int HttpThisMakeFullPath(char *filename, char *fullPath, int lengthFullPath); //Converts a relative file name to a full path by prepending the www folder