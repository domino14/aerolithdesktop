// this file is Mr. Mark Wutka's. Thanks!


#include <stdio.h>
#include <ctype.h>

#define MAX_WORD_LEN    30
#define MAX_CHAR_VALUE    256

typedef struct DawgNode
{
    char letter;
    char isEndOfWord;
    struct DawgNode *children;
    struct DawgNode *next;
    struct DawgNode *prev;
    int numChildren;
    int childDepth;
    int offset;
    char written;
    struct DawgNode *redirect;
    struct DawgNode *redirectNext;
    char visiting;
} DawgNode;

DawgNode **nodeTable;
DawgNode *redirNodes[MAX_WORD_LEN+1][MAX_CHAR_VALUE];

int numNodes = 0;

int offset = 0;

void addWord(char *word, int offset, int wordLen, DawgNode **root)
{
    DawgNode *prevNode, *currNode, *newNode;

    prevNode = 0;

    if (!(*root))
    {
        *root = (DawgNode *) malloc(sizeof(DawgNode));
        (*root)->letter = word[offset];
        (*root)->isEndOfWord = 0;
        (*root)->children = 0;
        (*root)->next = 0;
        (*root)->prev = 0;
        (*root)->offset = -1;
        (*root)->written = 0;
        (*root)->redirect = 0;
        (*root)->redirectNext = 0;
        (*root)->visiting = 0;

        currNode = *root;

        numNodes++;
    }
    else
    {
        currNode = *root;

        while (currNode)
        {
            if (currNode->letter == word[offset]) break;

            if (word[offset] < currNode->letter)
            {
                newNode = (DawgNode *) malloc(sizeof(DawgNode));
                newNode->letter = word[offset];
                newNode->isEndOfWord = 0;
                newNode->children = 0;
                newNode->next = 0;
                newNode->offset = -1;
                newNode->written = 0;
                newNode->redirect = 0;
                newNode->redirectNext = 0;
                newNode->visiting = 0;

                newNode->prev = prevNode;
                numNodes++;

                if (!prevNode)
                {
                    *root = newNode;
                }
                else
                {
                    prevNode->next = newNode;
                }
                break;
            }

            prevNode = currNode;
            currNode = currNode->next;
        }

        if (!currNode)
        {
            currNode = (DawgNode *) malloc(sizeof(DawgNode));
            currNode->letter = word[offset];
            currNode->children = 0;
            currNode->next = 0;
            currNode->prev = 0;
            currNode->isEndOfWord = 0;
            currNode->offset = -1;
            currNode->written = 0;
            currNode->redirect = 0;
            currNode->redirectNext = 0;
            currNode->visiting = 0;

            prevNode->next = currNode;
            numNodes++;
        }
    }

    if (offset == wordLen-1)
    {
        currNode->isEndOfWord = 1;
    }
    else
    {
        addWord(word, offset+1, wordLen, &currNode->children);
    }
}

void computeOffsets(DawgNode *inStartNode, int depth)
{
    DawgNode *currNode, *startNode;

    if (!inStartNode) return;

    if (inStartNode->offset >= 0) return;
    
    startNode = inStartNode;

    while (startNode->prev) startNode = startNode->prev;

    if (depth > 0)
    {
        currNode = startNode;

        while (currNode)
        {
            currNode->offset = offset;
            offset++;
            currNode = currNode->next;
        }
    }

    currNode = startNode;

    while (currNode)
    {
        if (currNode->children && currNode->children->redirect)
        {
            computeOffsets(currNode->children->redirect, depth+1);
        }
        else
        {
            computeOffsets(currNode->children, depth+1);
        }
        currNode = currNode->next;
    }

    if (depth == 0)
    {
        currNode = startNode;
        while (currNode)
        {
            currNode->offset = offset;
            offset++;
            currNode = currNode->next;
        }
    }
}

void writeNodeEntry(FILE *outfile, DawgNode *node)
{
    int outValue;
    unsigned char bytes[4];

    if (node->children)
    {
        if (node->children->redirect)
        {
            outValue = node->children->redirect->offset * 1024;
        }
        else
        {
            outValue = node->children->offset * 1024;
        }
    }
    else
    {
        outValue = 0;
    }

    outValue = outValue + node->letter;
    if (node->isEndOfWord)
    {
        outValue = outValue + 256;
    }

    if (!node->next)
    {
        outValue = outValue + 512;
    }

    bytes[0] = (outValue >> 24) & 255;
    bytes[1] = (outValue >> 16) & 255;
    bytes[2] = (outValue >> 8) & 255;
    bytes[3] = outValue & 255;
    
    //printf("%d ", outValue);
    fwrite(bytes, 1, 4, outfile);
}

void writeNode(FILE *outfile, DawgNode *inStartNode, int depth)
{
    DawgNode *currNode, *startNode;
    int numWritten;
    unsigned char bytes[4];

    if (!inStartNode) return;

    if (inStartNode->written) return;

    startNode = inStartNode;

    while (startNode->prev) startNode = startNode->prev;

    if (depth > 0)
    {
        currNode = startNode;

        while (currNode)
        {
            writeNodeEntry(outfile, currNode);
            currNode->written = 1;
            currNode = currNode->next;
        }
    }

    currNode = startNode;

    while (currNode)
    {
        if (currNode->children && currNode->children->redirect)
        {
            writeNode(outfile, currNode->children->redirect, depth+1);
        }
        else
        {
            writeNode(outfile, currNode->children, depth+1);
        }

        currNode = currNode->next;
    }

    numWritten = 0;
    if (depth == 0)
    {
        currNode = startNode;

        while (currNode)
        {
            writeNodeEntry(outfile, currNode);
            currNode->written = 1;
            currNode = currNode->next;
            numWritten++;
        }

        memset(bytes, 0, 4);
        while (numWritten < 26)
        {

	  printf("herE");
            fwrite(bytes, 1, 4, outfile);
            numWritten++;
        }
    }
}

int redirLen(DawgNode *node)
{
    int len = 0;

    while (node)
    {
        len++;
        node = node->redirectNext;
    }
}

void updateCounts(DawgNode *startNode)
{
    DawgNode *currChild, *currNode;

    currNode = startNode;

    while (currNode)
    {
        currNode->numChildren = 0;
        currNode->childDepth = 0;

        nodeTable[numNodes++] = currNode;

        updateCounts(currNode->children);

        currChild = currNode->children;

        while (currChild)
        {
            currNode->numChildren += currChild->numChildren;
            if (currChild->childDepth > currNode->childDepth)
            {
                currNode->childDepth = currChild->childDepth;
            }
            currChild = currChild->next;
        }

        currNode->childDepth++;

        currNode->redirectNext = redirNodes[currNode->childDepth][currNode->letter];
        redirNodes[currNode->childDepth][currNode->letter] = currNode;

        currNode = currNode->next;
    }
}

int nodesAreEquivalent(DawgNode *node1, DawgNode *node2)
{
    if (node1 == node2) return 1;

    if (!node1 || !node2) return 0;

    if ((node1->letter != node2->letter) ||
        (node1->isEndOfWord != node2->isEndOfWord) ||
        (node1->numChildren != node2->numChildren) ||
        (node1->childDepth != node2->childDepth)) return 0;

    return nodesAreEquivalent(node1->children, node2->children) &&
        nodesAreEquivalent(node1->next, node2->next);
}

void combineNodes()
{
    DawgNode *currNode, *currCompareNode;
    int i, j;

    for (i=0; i < MAX_WORD_LEN; i++)
    {
        printf("%d..", i); fflush(stdout);

        for (j=0; j < MAX_CHAR_VALUE; j++)
        {
            if (isalpha(j)) printf("%c", j); fflush(stdout);

            currNode = redirNodes[i][j];

            while (currNode)
            {
                if (currNode->redirect)
                {
                    currNode = currNode->redirectNext;
                    continue;
                }
                if (!currNode) break;

                currCompareNode = currNode->redirectNext;

                while (currCompareNode)
                {
                    if (currCompareNode->redirect)
                    {
                        currCompareNode = currCompareNode->redirectNext;
                        continue;
                    }
                    if (!currCompareNode) break;

                    if (nodesAreEquivalent(currNode, currCompareNode))
                    {
                        currCompareNode->redirect = currNode;
                    }
                    currCompareNode = currCompareNode->redirectNext;
                }
                currNode = currNode->redirectNext;
            }
        }
    }
    printf("\n");
}

int main(int argc, char *argv[])
{
    FILE *infile, *outfile;
    DawgNode *root;
    char line[1024];
    int i, j, len, numWords;
    unsigned char bytes[4];

    if (argc < 3)
    {
        fprintf(stderr, "Format is: %s input-word-list-file output-dawg-file\n",
                argv[0]);
        exit(1);
    }

    if (!(infile = fopen(argv[1], "r")))
    {
        fprintf(stderr, "Can't open %s\n", argv[1]);
        perror("fopen");
        exit(1);
    }

    if (!(outfile = fopen(argv[2], "w")))
    {
        fprintf(stderr, "Can't open %s\n", argv[2]);
        perror("fopen");
        exit(1);
    }

    root = 0;

    numWords = 0;

    while (fgets(line, sizeof(line)-1, infile))
    {
        len = strlen(line)-1;
        while ((len > 0) && isspace(line[len]))
        {
            line[len--] = 0;
        }
        if (len <= 0) continue;
        len++;
        
        for (i=0; i < len; i++)
        {
            line[i] = tolower(line[i]);
        }
        if (len <= MAX_WORD_LEN)
        {
            addWord(line, 0, len, &root);
            numWords++;
        }
    }
    fclose(infile);

    nodeTable = (DawgNode **) malloc((numNodes+1) * sizeof(DawgNode *));

    for (i=0; i < numNodes; i++)
    {
        nodeTable[i] = 0;
    }

    for (i=0; i < MAX_WORD_LEN; i++)
        for (j=0; j < MAX_CHAR_VALUE; j++)
            redirNodes[i][j] = 0;

    printf("Read %d words into %d nodes\n", numWords, numNodes);

    numNodes = 0;

    printf("Updating depth/children counts\n");
    updateCounts(root);

    printf("Combining nodes\n");
    combineNodes();

    offset = 1;

    printf("Computing offsets\n");
    computeOffsets(root, 0);

    printf("Writing dawg to file\n");

    bytes[0] = 0;
    bytes[1] = 0;
    bytes[2] = 3;
    bytes[3] = 0;

    fwrite(bytes, 1, 4, outfile);

    writeNode(outfile, root, 0);

    fclose(outfile);
}
