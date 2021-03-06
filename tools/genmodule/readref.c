/*
    Copyright � 1995-2005, The AROS Development Team. All rights reserved.
    $Id$
    
    Function to read in the function reference file. Part of genmodule.
*/
#include "genmodule.h"
#include "fileread.h"
#include <assert.h>

/* A structure for storing parse state information during scanning of the ref file */
struct _parseinfo
{
    char infunction;
    struct stringlist *currreg;
    struct functionhead *currentfunc;
};

/* Prototypes of static functions */
static int parsemethodname(char *name,
			   struct _parseinfo *parseinfo,
			   struct config *cfg
);
static int parsemacroname(char *name,
			  struct _parseinfo *parseinfo,
			  struct config *cfg
);

void readref(struct config *cfg)
{
    struct functionhead *funclistit = NULL;
    struct functionhead *currentfunc = NULL; /* will be either funclistit or methlistit */
    struct _parseinfo parseinfo;
    unsigned int funcnamelength;
    char *begin, *end, *line;

    if (!fileopen(cfg->reffile))
    {
	fprintf(stderr, "In readref: Could not open %s\n", cfg->reffile);
	exit(20);
    }

    /* Go over all the lines in the ref file and look for the functions and their
     * arguments in the file. There are two states during parsing infunction==0 for
     * outside a function header and then just look for the next function name.
     * infunction==1 in a function header and then the arguments of this function
     * are parsed.
     */
    parseinfo.infunction = 0;
    while ((line=readline())!=NULL)
    {
	if (strlen(line)>0)
	{
	    /* When the registers are specified in the config file check if the number provided
	     * provided in the file corresponds with the number of arguments in the ref file
	     */
	    if (parseinfo.infunction &&
		(strncmp(line, "FILE", 4)==0 ||
		 strncmp(line, "INCLUDES", 8)==0 ||
		 strncmp(line, "DEFINES", 7)==0 ||
		 strncmp(line, "VARIABLE", 8)==0 ||
		 strncmp(line, "FUNCTION", 8)==0
		) &&
		parseinfo.currentfunc->libcall==REGISTER)
	    {
		/* About to leave function */
		if (parseinfo.currreg!=NULL)
		{
		    fprintf(stderr, "Error: too many registers specified for function \"%s\"\n",
			    parseinfo.currentfunc->name);
		    exit(20);
		}
	    }

	    /* End of function header ? */
	    if (parseinfo.infunction
		&& (strncmp(line, "FILE", 4)==0
		    || strncmp(line, "INCLUDES", 8)==0
		    || strncmp(line, "DEFINES", 7)==0
		    || strncmp(line, "VARIABLE", 8)==0
		)
	    )
		parseinfo.infunction = 0;

	    /* Start of a new function header ? */
	    if (strncmp(line, "FUNCTION", 8)==0)
	    {
		begin = strchr(line,':');
		if (begin==NULL)
		{
		    fprintf(stderr, "Syntax error in reffile %s\n", cfg->reffile);
		    exit(20);
		}
		begin++;
		while (isspace(*begin)) begin++;
		end = strchr(begin, '[');
		if (end==NULL)
		{
		    fprintf(stderr, "Syntax error in reffile %s\n", cfg->reffile);
		    exit(20);
		}
		while (isspace(*(end-1))) end--;
		*end = '\0';

		funcnamelength = strlen(begin);
		
		parseinfo.infunction =
		(
		       parsemethodname(begin, &parseinfo, cfg)
		    || parsemacroname(begin, &parseinfo, cfg)
		);
	    }
	    else if (parseinfo.infunction)
	    {
		if (strncmp(line, "Arguments", 9)==0)
		{
		    int i, brackets=0;
		    char *name;
		    
		    begin = strchr(line, ':');
		    if (begin==NULL)
		    {
			fprintf(stderr, "Syntax error in reffile %s\n", cfg->reffile);
			exit(20);
		    }
		    begin++;
		    while (isspace(*begin)) begin++;

		    /* for libcall == STACK the whole argument is the type
		     * otherwise split the argument in type and name
		     */
#if 0
		    if (parseinfo.currentfunc->libcall != STACK)
		    {
			/* Count the [] specification at the end of the argument */
			end = begin+strlen(begin);
			while (isspace(*(end-1))) end--;
			while (*(end-1)==']')
			{
			    brackets++;
			    end--;
			    while (isspace(*(end-1))) end--;
			    if (*(end-1)!='[')
			    {
				fprintf(stderr,
					"Argument \"%s\" not understood for function %s\n",
					begin, parseinfo.currentfunc->name
				);
				exit(20);
			    }
			    end--;
			    while (isspace(*(end-1))) end--;
			}
			*end='\0';
			
			/* Skip over the argument name and duplicate it */
			while (!isspace(*(end-1)) && *(end-1)!='*') end--;
			name = strdup(end);

			/* Add * for the brackets */
			while (isspace(*(end-1))) end--;
			for (i=0; i<brackets; i++)
			{
			    *end='*';
			    end++;
			}
			*end='\0';
		    }
#endif
		    if (strcasecmp(begin, "void")!=0)
		    {
			switch (parseinfo.currentfunc->libcall)
			{
			case STACK:
			    funcaddarg(parseinfo.currentfunc, begin, NULL);
			    break;
			case REGISTER:
			    if (parseinfo.currreg == NULL)
			    {
				fprintf(stderr,
					"Error: argument count mismatch for funtion \"%s\"\n",
					parseinfo.currentfunc->name
				);
				exit(20);
			    }
			    funcaddarg(parseinfo.currentfunc, begin, parseinfo.currreg->s);
			    parseinfo.currreg = parseinfo.currreg->next;
			    break;
			case REGISTERMACRO:
			    {
				char *reg = begin + strlen(begin) - 1;
				while (reg != begin && *reg != '_') reg--;
		
				assert(reg > begin);
				
				*reg = '\0';
				reg++;

				funcaddarg(parseinfo.currentfunc, begin, reg);
			    }
			    break;
			default:
			    fprintf(stderr,
				    "Internal error: Unhandled modtype in readref\n"
			    );
			    break;
			}
		    }
		}
		else if (strncmp(line, "Type", 4)==0)
		{
		    begin = strchr(line, ':');
		    if (begin==NULL)
		    {
			fprintf(stderr, "Syntax error in reffile %s\n", cfg->reffile);
			exit(20);
		    }
		    begin++;
		    while (isspace(*begin)) begin++;
		    end = begin+strlen(begin)-funcnamelength;
		    while (isspace(*(end-1))) end--;
		    *end = '\0';
		    parseinfo.currentfunc->type = strdup(begin);
		}
	    }
	}
    };
    fileclose();

    /* Checking to see if every function has a prototype */
    for 
    (
        funclistit = cfg->funclist; 
        funclistit != NULL; 
        funclistit =  funclistit->next
    )
    {
	if (funclistit->type==NULL)
	{
	    fprintf
            (   
		stderr, "Did not find function %s in reffile %s\n", 
		funclistit->name, cfg->reffile
            );
	    exit(20);
	}
    }
}


static int parsemethodname(char *name,
			   struct _parseinfo *parseinfo,
			   struct config *cfg
)
{
    int ok = 0;
    struct classinfo *cl;
    
    for (cl = cfg->classlist; cl != NULL; cl = cl->next)
    {
	char *sep = NULL;
	const char **prefixptr = cl->boopsimprefix;
	
	/* For a BOOPSI class a custom dispatcher has the name
	 * 'modulename_Dispatcher'
	 */
	if
	(
	       strncmp(name, cl->basename, strlen(cl->basename)) == 0
	    && strcmp(name+strlen(cl->basename), "_Dispatcher") == 0
	)
	    cl->dispatcher = strdup(name);	

	while (*prefixptr != NULL && sep == NULL)
	{
	    sep = strstr(name, *prefixptr);
	    prefixptr++;
	}
                
	if 
	(
	       sep != NULL
	    && strncmp(cl->basename, name, sep - name) == 0
	)
	{
	    struct functionhead *method, *it;

	    method = newfunctionhead(sep+2, STACK);
	    
	    if (cl->methlist == NULL )
		cl->methlist = method;
	    else
	    {
		it = cl->methlist;
		
		while (it->next != NULL) it = it->next;
		
		it->next = method;
	    }
                    
	    parseinfo->currentfunc = method;
	    ok = 1;
	}
    }

    return ok;
}


static int parsemacroname(char *name,
			  struct _parseinfo *parseinfo,
			  struct config *cfg
)
{
    if
    (
        strncmp(name, "AROS_LH_", 8)    == 0
        || strncmp(name, "AROS_PLH_", 9)   == 0
        || strncmp(name, "AROS_NTLH_", 10) == 0
    )
    {
	struct functionhead *funclistit, *func;
	char *begin, *end, *funcname;
	int novararg = 0, priv = 0;
	unsigned int lvo;
	
	if (strncmp(name, "AROS_LH_", 8) == 0)
	{
	    begin = name+8;
	}
	else
	if (strncmp(name, "AROS_PLH_", 9) == 0)
	{
	    begin = name+9;
	    priv = 1;
	}
	else
	{
	    begin = name+10;
	    novararg = 1;
	}
	
	if (strncmp(begin, cfg->basename, strlen(cfg->basename)) != 0
	    || begin[strlen(cfg->basename)] != '_'
	)
	    return 0;
	
	begin = begin + strlen(cfg->basename) + 1;
	end = begin + strlen(begin) - 1;
	
	while(end != begin && *end != '_') end--;
	*end = '\0';
	funcname = begin;
	
	begin = end+1;
	sscanf(begin, "%d", &lvo);

	func = newfunctionhead(funcname, REGISTERMACRO);
	func->lvo      = lvo;
	func->novararg = novararg;
	func->priv     = priv;

	if (cfg->funclist == NULL || cfg->funclist->lvo > func->lvo)
	{
	    func->next = cfg->funclist;
	    cfg->funclist = func;
	}
	else
	{
	    for
	    (
	        funclistit = cfg->funclist;
	        funclistit->next != NULL && funclistit->next->lvo < func->lvo;
	        funclistit = funclistit->next
	    )
		;
	 
	    if (funclistit->next != NULL && funclistit->next->lvo == func->lvo)
	    {
		fprintf(stderr,
			"Function '%s' and '%s' have the same LVO number\n",
			funclistit->next->name, func->name
		);
		exit(20);
	    }
		
	    func->next = funclistit->next;
	    funclistit->next = func;
	}

	parseinfo->currentfunc = func;
	
	return 1;
    }
    else
	return 0;
}
