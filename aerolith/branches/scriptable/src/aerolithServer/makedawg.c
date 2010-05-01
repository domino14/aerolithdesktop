
//    Copyright 1999, 2009 Mark Wutka <mark@wutka.com>
//    This file is part of Aerolith. It was released under the GPL by Mark Wutka
//    and modified by Cesar Del Solar. The modifications are marked in the source
//    with "cesar". This file is not meant to be compiled with Aerolith, it is a
//    standalone dawg-maker. 
//
//    Aerolith is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    Aerolith is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with Aerolith.  If not, see <http://www.gnu.org/licenses/>.

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

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
	    /*	    else
		    fprintf(stderr, "something is wrong %c %c\n", word[offset], currNode->letter);*/
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
/* cesar: modified computeOffsets to return the number of beginning letters. */
int computeOffsets(DawgNode *inStartNode, int depth)
{
    DawgNode *currNode, *startNode;
    int numBeginningLetters;
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
    numBeginningLetters = 0;
    if (depth == 0)
    {
        currNode = startNode;
        while (currNode)
        {
            currNode->offset = offset;
            offset++;
            currNode = currNode->next;
	    numBeginningLetters++;
        }
    }
    return numBeginningLetters;
}

/* cesar: added transform5bit, geared towards the spanish lexicon */
int transform5bit(unsigned char letter)
{
  if (letter >= 'a' && letter <= 'z')
    return letter - 'a';
  if (letter >= 'A' && letter <= 'z')
    return letter - 'A';
  if (letter >= '1' && letter <= '4')
    return letter - '1' + 26; /* so 26, 27, 28, or 29*/
}

void writeNodeEntry(FILE *outfile, DawgNode *node)
{
    int outValue;
    unsigned char bytes[4];

    if (node->children)
    {
        if (node->children->redirect)
        {
            outValue = node->children->redirect->offset * 128;
        }
        else
        {
            outValue = node->children->offset * 128;
        }
    }
    else
    {
        outValue = 0;
    }

    /* cesar: modified the below line from outValue = outValue + node->letter */
    outValue = outValue + transform5bit(node->letter);
    if (node->isEndOfWord)
    {
        outValue = outValue + 32;
    }

    if (!node->next)
    {
        outValue = outValue + 64;
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
    int numBeginningLetters;
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
	/* cesar: added below for loop. this is so the dawg can take in files with definitions. words
	 must still be sorted tho.*/
        for (i=0; i < len; i++)
        {
            line[i] = tolower(line[i]);
	    if (line[i] == ' ')
	      len = i;
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
    numBeginningLetters = computeOffsets(root, 0);
    /* Cesar: these are the number of beginning letters .. that is, the number of letters that begin words.
       should be 26 for most dictionaries but its 27 for spanish :) */

    printf("Writing dawg to file\n");

    /* cesar: modifying the format here, but who cares -- i'm not using this for the scrabble cd-rom*/
    bytes[0] = 0;
    bytes[1] = 0;
    bytes[2] = 0;
    bytes[3] = numBeginningLetters;

    fwrite(bytes, 1, 4, outfile);

    writeNode(outfile, root, 0);

    fclose(outfile);
}
