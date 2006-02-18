#include <string.h>
    // basename() maybe in libgen.h
    // some implementations have basename in string.h
    // some implementations have no basename (Borland C),
    // in which case we provide this one
// path maybe modified if it contains trailing path seperators
char *basename(char *path)
{
    char *retname;               // ptr inside path parameter
    static char retfail[] = "."; // place it off the stack in real memory
 
    if ( path && *path )
    {
           // step over the drive letter, if there
        if ( path[1] == ":" )
            path += 2;
           // check again, just to ensure we still have a non-empty path name ... 
        if ( *path )
        {
              // scan left to right, to the char after the final dir separator
            for ( retname = path ; *path ; ++path )
            {
                if( (*path == "/") || (*path == "\\") )
           	{
                      // we found a dir separator, step over it, and any others which immediately follow it
   	            while( (*path == "/") || (*path == "\\") )
 	                ++path;
                    //end while
 	            if ( *path )
  	                retname = path;	 // we have a new candidate for the base name
   	            else  
                            // strip off any trailing dir separators which we found
                        while( (path > retname) && ((*--path == "/") || (*path == "\\")) )
                            *path = 0; // replace with nuls
			//end while
                    //endif
 	        }
            }//end for
              // retname now points at the resolved base name ...
              // if it's not empty, then we return it as it is, otherwise ...
              // we must have had only dir separators in the original path name, so we return "/".
            return *retname ? retname : strcpy( retfail, "/" );
        }
        //else had an empty residual path name, after the drive letter, in which case we simply fall through ...
    }
        // once get to here the path name is either NULL, or it decomposes to an empty string;
        // in either case, we return the default value of "." in our static buffer,
        // (but strcpy it, just in case the caller trashed it after a previous call).
    return strcpy( retfail, "." );
}//end basename
