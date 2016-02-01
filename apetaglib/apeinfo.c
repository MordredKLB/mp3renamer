#include <ansi_c.h>
#include <utility.h>
#include <apetag.h>
//#include <err.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "apev2.h"

void ApeItem_print(struct ApeItem *item);


/* Print out the items in the file */
int ApeInfo_process(char *filename, struct ApeTag **tag) {
	int ret;
	int status;
	FILE *file = NULL;
	
	*tag = NULL;

	if ((file = fopen(filename, "rb")) == NULL) {
		ErrorPrintf("%s", filename);
	    ret = 1;
	    goto apeinfo_process_error;
	}

	if ((*tag = ApeTag_new(file, 0)) == NULL) {
	    ErrorPrintf(NULL);
	    ret = 1;
	    goto apeinfo_process_error;
	}

	ApeTag_set_filename(*tag, filename);

	status = ApeTag_parse(*tag);
	if (status == -1) {
	    ErrorPrintf("%s: %s", filename, ApeTag_error(*tag));
	    ret = 1;
	    goto apeinfo_process_error;
	}
//	ApeInfo_open_tag(filename, &tag, "rb");	// TODO: Figure out why I can't use this and close tag->file below

	if (ApeTag_exists(*tag)) {
	//    ErrorPrintf("%s (%i items):\n", filename, ApeTag_item_count(*tag));
	//    ApeTag_print(*tag);
	} else {
		ErrorPrintf("%s: no ape tag\n\n", filename);
	}
	
	ret = 0;
    
apeinfo_process_error:
	if (file != NULL) {
		if (fclose(file) != 0) {
		    ErrorPrintf("%s", filename);
		}					
	}
    
    return ret;
}

int ApeInfo_open_tag(char *filename, struct ApeTag **tag, char *mode) 
{
	int ret;
	int status;
	FILE *file = NULL;
	
	*tag = NULL;

	if ((file = fopen(filename, mode)) == NULL) {
		ErrorPrintf("%s", filename);
	    ret = 1;
	    goto apeinfo_process_error;
	}

	if ((*tag = ApeTag_new(file, 0)) == NULL) {
	    ErrorPrintf(NULL);
	    ret = 1;
	    goto apeinfo_process_error;
	}
	
	ApeTag_set_filename(*tag, filename);

	status = ApeTag_parse(*tag);
	if (status == -1) {
	    ErrorPrintf("%s: %s", filename, ApeTag_error(*tag));
	    ret = 1;
	    goto apeinfo_process_error;
	}
	
	ret = 0;

apeinfo_process_error:
	if (ret) {		// close file if error
		if (file != NULL) {
			if (fclose(file) != 0) {
			    ErrorPrintf("%s", filename);
			}
		}
	}
	return ret;
}

void ApeTag_close(struct ApeTag *tag) {
	ApeTag_free(tag);
}

int ApeTag_iter_print(struct ApeTag *tag, struct ApeItem *item, void *data) {
	ApeItem_print(item);
	return 0;	// ApeItem_print does not return an error code
}

/* Prints all items in the tag, one per line. */
void ApeTag_print(struct ApeTag *tag) {
    assert(tag != NULL);

    if (ApeTag_iter_items(tag, ApeTag_iter_print, NULL) < 0) {
       ErrorPrintf("Error getting items: %s", ApeTag_error(tag));
    }
}

/* 
Prints a line with the key and value of the item separated by a colon. Includes
information about the tags flags unless they are the default (read-write UTF8).
*/
void ApeItem_print(struct ApeItem *item) {
    int i;
    char c;
    
    assert(item != NULL);
    assert(item->key != NULL);
    assert(item->value != NULL);

    printf("%s: ", item->key);
    if ((item->flags & APE_ITEM_TYPE_FLAGS) == APE_ITEM_BINARY) {
        printf("[BINARY DATA]");
    } else if ((item->flags & APE_ITEM_TYPE_FLAGS) == APE_ITEM_RESERVED) {
        printf("[RESERVED]");
    } else {
        if ((item->flags & APE_ITEM_TYPE_FLAGS) == APE_ITEM_EXTERNAL) {
            printf("[EXTERNAL LOCATION] ");
        }
        for (i=0; i < item->size; i++) {
            c = *((char *)(item->value)+i);
            if (c == '\0') {
                printf(", ");
            } else if (c < '\40') {
                printf("\\%o", c);
            } else if (c == '\\') {
                printf("\\\\");
            } else {
                printf("%c", c);
            }
        }
    }
    if (item->flags & APE_ITEM_READ_ONLY) {
        printf(" [READ_ONLY]");
    }
    printf("\n");
}
