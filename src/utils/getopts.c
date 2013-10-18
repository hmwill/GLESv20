/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the public domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  getopts - scan the command line for switches.                    */
/*                                                                   */
/*  This program takes the following parameters:                     */
/*                                                                   */
/*  1) argc (which was given to main)                                */
/*  2) argv (which was given to main)                                */
/*  3) Array of options                                              */
/*                                                                   */
/*  Returns the number of the argument that is next to be processed  */
/*  that wasn't recognised as an option.                             */
/*  Example of use:                                                  */
/*                                                                   */
/*  #include <getopts.h>                                             */
/*  int baud = 2400;                                                 */
/*  char fon[13] = "telix.fon";                                      */
/*  opt_t opttable[] =                                               */
/*  {                                                                */
/*    { "b", OPTINT, &baud },                                        */
/*    { "f", OPTSTR, fon },                                          */
/*    { NULL, 0, NULL }                                              */
/*  }                                                                */
/*  optup = getopts(argc,argv,opttable);                             */
/*                                                                   */
/*  The OPTINT means that an integer is being supplied.  OPTSTR      */
/*  means a string (with no check for overflow).  Also there is      */
/*  OPTBOOL which means it is a switch that is being passed, and an  */
/*  OPTLONG to specify a long.  Also OPTFLOAT for float.             */
/*                                                                   */
/*  This program was inspired by a description of a getargs function */
/*  written by Dr Dobbs Small-C Handbook.  Naturally I didn't get    */
/*  to see the code, otherwise I wouldn't be writing this!           */
/*                                                                   */
/*  This program is dedicated to the public domain.  It would be     */
/*  nice but not necessary if you gave me credit for it.  I would    */
/*  like to thank the members of the International C Conference      */
/*  (in Fidonet) for the help they gave me in writing this.          */
/*                                                                   */
/*  Written 16-Feb-1990.                                             */
/*                                                                   */
/*********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "Utils.h"

GLint UtilGetOpts(GLint argc, char **argv, UtilOption opttable[])
{
  GLint i,j;
  argv++;
  argc--;
  for (i=1;i<=argc;i++)
  {
    if ((*(*argv) != '-') && (*(*argv) != '/')) return (i);
    for (j=0;opttable[j].sw != NULL;j++)
    {
      if (strncmp(*argv+1,opttable[j].sw,
          strlen(opttable[j].sw)) == 0)
      {
        switch (opttable[j].opttyp)
        {
          case UtilOptionInt :
            *((GLint *)opttable[j].var) =
                (GLint)strtol(*argv+1+strlen(opttable[j].sw), NULL, 10);
            if (errno == ERANGE) return (i);
            break;
          case UtilOptionString:
            strcpy(opttable[j].var, *argv+1+strlen(opttable[j].sw));
            break;
          case UtilOptionBoolean:
            *((GLboolean *)opttable[j].var) = 1;
            break;
          case UtilOptionFloat:
            *((GLfloat *)opttable[j].var) =
                (GLfloat)strtod(*argv+1+strlen(opttable[j].sw), NULL);
            break;
		  default:
			break;
        }
        break;
      }
    }
    argv++;
  }
  return (i);
}
