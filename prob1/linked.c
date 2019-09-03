#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structs for nodes and edges
struct node {
	char* name;
	int visited;
	struct edge* linkHead;
	struct node* next;
};

struct edge {
	struct node* to;
	struct edge* next;
};

// Function prototypes:
int validCommand(char*);
void addPages(char*);
void addLinks(char*);
int isConnected(char*);
int dfs(struct node*, struct node*);
void freeList(struct node*);
void outOfMemory();

// Global variable: page list header
struct node* head = NULL;

// Global variable: error seen
int errSeen = 0;

int main(int argc, char*argv[]) {
	FILE* input = NULL;
	
	// Set input stream/open file
	if (argc <= 1) {
		input = stdin;
	} else if (argc == 2) {
		input = fopen(argv[1], "r");
		if (input == NULL) {
			fprintf(stderr, "%s: No such file or directory", argv[1]);
			exit(1);
		}
	} else {
		fprintf(stderr, "Too many arguments sent to %s", argv[0]);
		errSeen = 1;
	}
	
	char* line = NULL;
	size_t len = 0;
	// Read in a line fron input stream
	while (	getline(&line, &len, input) != EOF) {
		if (line == NULL) {
			outOfMemory();
		}
		char* linePtr = line;
		char str[65];
		int isLineStart = 1;
		while ( sscanf(linePtr, "%64s", str) > 0 ) {
			linePtr += strlen(str) + 1;
			if (isLineStart) {
				// look at first word of line
				// execute command or generate error if not a command
				char* linePtrDup = strdup(linePtr);
				if (!validCommand(str)) {
					fprintf(stderr, "Bad command: %s\n", str);
					errSeen = 1;
				} else if (strcmp(str, "@addPages") == 0) {
					addPages(linePtrDup);
				} else if (strcmp(str, "@addLinks") == 0) {
					addLinks(linePtrDup);
				} else if (strcmp(str, "@isConnected") == 0) {
					int connected = isConnected(linePtrDup);
					if (connected >= 0) {
						printf("%d\n", connected);
					}
				}
				isLineStart = 0;
				free(linePtrDup);
			}
			
		}
	}
/* 	struct node* ptr = head;
	printf("PRINTING PAGE LIST:\n");
	while (ptr) {
		struct edge* linkPtr = ptr->linkHead;
		printf("Page: %s\tLinked to: ", ptr->name);
		while (linkPtr) {
			printf("%s ", linkPtr->to->name);
			linkPtr = linkPtr->next;
		}
		printf("\n");
		ptr = ptr->next;
	} */
	
	free(line);
	freeList(head);
	fclose(input);
	return errSeen;
}

void outOfMemory() {
	fprintf(stderr, "Out of memory\n");
	freeList(head);
	exit(1);
}

void freeList(struct node* head) {
	struct edge* linkPtr;
	struct edge* nextPtr;
	struct node* nextNode;
	while (head != NULL) {
		free(head->name);
		linkPtr = head->linkHead;
		while (linkPtr != NULL) {
			//free(linkPtr->to);
			nextPtr = linkPtr->next;
			free(linkPtr);
			linkPtr = nextPtr;
		}
		nextNode = head->next;
		free(head);
		head = nextNode;
	}	
}

int validCommand(char* str) {
	return (strcmp(str, "@addPages") == 0 || strcmp(str, "@addLinks") == 0 || strcmp(str, "@isConnected") == 0);
}

// Takes a char* with a (non-empty) whitespace-seperated list of page names to create
// Parses name list and creates nodes for each page name
void addPages(char* pages) {
	char pageName[65];
	char* pagesPtr = pages;
	while ( sscanf(pagesPtr, "%64s", pageName) > 0 ) {
		// Create new node
		struct node* newNode = malloc(sizeof(struct node));
			if (newNode == NULL) {
				outOfMemory();
			}
			newNode->name = strdup(pageName);
			newNode->visited = 0;
			newNode->linkHead = NULL;
			newNode->next = NULL;
		
		if (!head) {	// Head does not exist, set head to newNode
			head = newNode;
		} else {	// Head DOES exist, add newNode to end of list (if not already in list)
			struct node* nodePtr = head;
			int pageExists = 0;
			if (strcmp(head->name, pageName) == 0) {
				pageExists = 1;
			}
			while (nodePtr->next != NULL) {
				if (strcmp(nodePtr->name, pageName) == 0) {
					pageExists = 1;
				}
				nodePtr = nodePtr->next;
			}
			if (!pageExists) {
				nodePtr->next = newNode;
			} else {
				fprintf(stderr, "%s added twice\n", pageName);
				errSeen = 1;
				freeList(newNode);
			}
			
		}
		pagesPtr += strlen(pageName) + 1;
	}
}

// Takes a char* with a source page and a list of names to link to
// Parses list and creates links
void addLinks(char* links) {
	char sourcePage[65];
	char* linksPtr = links;
	// If no source page:
	if ( sscanf(linksPtr, "%64s", sourcePage) < 1) {
		fprintf(stderr, "No source page given\n");
		errSeen = 1;
		return;
	} else {
		// move linksPtr past sourcePage
		linksPtr += strlen(sourcePage) + 1;
		
		// move pagePtr to point to sourcePage node
		struct node* pagePtr = head;
		while (pagePtr) {
			if (strcmp(sourcePage, pagePtr->name) == 0) {
				break;
			}
			pagePtr = pagePtr->next;
		}
		struct node* sourcePtr = pagePtr;
		// If sourcePage was not found, report error and return
		if (sourcePtr == NULL) {
			fprintf(stderr, "Source page %s doesn't exist\n", sourcePage);
			errSeen = 1;
			return;
		}
		
		// Parse through the rest of linksPtr, adding links to sourcePtr
		char linkToPage[65];
		while ( sscanf(linksPtr, "%64s", linkToPage) > 0 ) {
			// Check whether linkToPage node exists
			pagePtr = head;
			while (pagePtr) {
				if (strcmp(linkToPage, pagePtr->name) == 0) {
					break;
				}
				pagePtr = pagePtr->next;
			}
			struct node* linkToPtr = pagePtr;
			// Page to link to was not found, continue to next link in list
			if (linkToPtr == NULL) {
				fprintf(stderr, "Target page %s doesn't exist\n", linkToPage);
				errSeen = 1;
				linksPtr += strlen(linkToPage) + 1;
				continue;
			}
			
			// Add links to source page
			struct edge* newLink = malloc(sizeof(struct edge));
			if (newLink == NULL) {
				outOfMemory();
			}
			newLink->to = linkToPtr;
			newLink->next = NULL;
			// Source page has no links. Make new link head of links list
			if (!sourcePtr->linkHead) {
				sourcePtr->linkHead = newLink;
			} else {
				// add new link to end of link list (if not already there)
				struct edge* linkPtr = sourcePtr->linkHead;
				int linkExists = 0;
				while (linkPtr) {
					if (strcmp(linkPtr->to->name, linkToPage) == 0) {
						linkExists = 1;
					}
					linkPtr = linkPtr->next;
				}
				linkPtr = sourcePtr->linkHead;
				while (linkPtr->next != NULL) {
					linkPtr = linkPtr->next;
				}
				if (!linkExists) {
					linkPtr->next = newLink;
				} else {
					free(newLink);
				}
			}
			linksPtr += strlen(linkToPage) + 1;
		}
	}

}

// Parses input line. if valid, runs dfs to determine if two pages are connected
// if input is invalid, returns -1
int isConnected(char* line) {	
	char* linePtr = line;
	char page1[65];
	char page2[65];
	char dummyBuf[65];
	if ( sscanf(linePtr, "%64s", page1) < 1) {
		fprintf(stderr, "No from page given\n");
		errSeen = 1;
		return -1;
	}
	// Determine if first page exists
	struct node* pagePtr = head;
	struct node* fromPage = NULL;
	while (pagePtr) {
		if (strcmp(pagePtr->name, page1) == 0) {
			fromPage = pagePtr;
		}  
		pagePtr = pagePtr->next;
	}
	if (fromPage == NULL) {
		fprintf(stderr, "No page %s found\n", page1);
		errSeen = 1;
		return -1;
	}
	linePtr += strlen(page1) + 1;
	
	if ( sscanf(linePtr, "%64s", page2) < 1) {
		fprintf(stderr, "No target page given\n");
		errSeen = 1;
		return -1;
	}
	// Determine if second page exists
	pagePtr = head;
	struct node* toPage = NULL;
	while (pagePtr) {
		if (strcmp(pagePtr->name, page2) == 0) {
			toPage = pagePtr;
		}  
		pagePtr = pagePtr->next;
	}
	if (toPage == NULL) {
		fprintf(stderr, "No page %s found\n", page2);
		errSeen = 1;
		return -1;
	}
	linePtr += strlen(page2);
	

	// If there's anything else on the line after the two pages, error
	if ( sscanf(linePtr, "%64s", dummyBuf) > 0 ) {
		fprintf(stderr, "3Command was wrong\n");
		errSeen = 1;
		return -1;
	}
	
	// Determine if pages are connected
	int connected = dfs(fromPage, toPage);
	
	//Reset "visited" flags
	struct node* ptr = head;
	while (ptr) {
		ptr->visited = 0;
		ptr = ptr->next;
	}
	return connected;
}

int dfs(struct node* fromPage, struct node* toPage) {
	if (strcmp(fromPage->name, toPage->name) == 0) {
		return 1;
	}
	if (fromPage->visited) {
		return 0;
	}
	fromPage->visited = 1;
	struct edge* linkPtr;
	for (linkPtr = fromPage->linkHead; linkPtr != NULL; linkPtr = linkPtr->next) {
		if (dfs(linkPtr->to, toPage)) {
			return 1;
		}
	}
	return 0;
}