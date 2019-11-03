
// Standard includes
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <string.h>

// STL includes
#include <vector>

// The size of the hash space
static const uint64_t				HASH_SPACE_SIZE = 1UL << 32UL;

// The maximum number of characters in the string
static const size_t					MAX_ALLOWED_CHARS = 12;
static const unsigned int			MAX_ALLOWED_SEARCH = 100;

// All of the allowed characters
static const char					ALLOWED_CHARS [] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const size_t					NUM_ALLOWED_CHARS = strlen( ALLOWED_CHARS);

// A structure to hold working nodes
struct s_working_node {
	uint32_t	hashValue;
	size_t		strSize;
	char		strValue [MAX_ALLOWED_CHARS + 1];
};
typedef struct s_working_node		S_WORKING_NODE;

// Helpful typedefs
typedef unsigned char				BYTE;
typedef uint64_t					NODE_MAP_LEAF;
typedef	std::vector<S_WORKING_NODE>	CNT_NODES;
typedef	CNT_NODES::iterator			ITR_NODES;

//
// Compute a hash code
//

int getHashCode( const char *inpString) {

	// Init
	const char *pPos = inpString;
	uint32_t total = 0x0;

	// Loop over input string
	for( ; 0x0 != *pPos ; ++ pPos) {

		total *= 31;
		total += *pPos;

	}

	// And done
	return( (int) total);

}

//
// Is a particular node clear?
//
// WARNING! Function does not check arguments
// This is done to provide the fastest possible speed
//

bool nodeIsClear( NODE_MAP_LEAF *pMemory, uint32_t toCheck) {
	NODE_MAP_LEAF position = toCheck / (8 * sizeof( NODE_MAP_LEAF));
	NODE_MAP_LEAF mask = 1 << (toCheck % (8 * sizeof( NODE_MAP_LEAF)));
	uint64_t value = pMemory[position] & mask;
	return( 0x0 == value);
}

//
// Set a particular node
//

// WARNING! Function does not check arguments
// This is done to provide the fastest possible speed
//

void nodeSet( NODE_MAP_LEAF *pMemory, uint32_t toSet) {
	NODE_MAP_LEAF position = toSet / (8 * sizeof( NODE_MAP_LEAF));
	NODE_MAP_LEAF mask = 1 << (toSet % (8 * sizeof( NODE_MAP_LEAF)));
	pMemory[position] |= mask;
}

//////////
// MAIN //
//////////

int main( int argc, char * argv []) {

	// Variables
	bool			keepLooping = true;
	uint64_t		countProcessed = 0x0;
	uint64_t		countRecorded = 0x0;
	unsigned int	numFound = 0x0;
	unsigned int	NUM_TO_MATCH = 0x1;

	// Is there at least one argument?
	if( 2 > argc) {
		printf( "Usage error: %s <string to hash> [<number of hashes>]\n", argv [0]);
		exit( EXIT_FAILURE);
	}

	// Get the hash code to look for
	const char * STR_TO_CHECK = argv [1];
	const uint32_t HASH_TO_CHECK = (uint32_t) getHashCode( STR_TO_CHECK);
	printf( "Looking for hash: %d\n", (int) HASH_TO_CHECK);

	// Change the number to match?
	if( 3 >= argc) {
		int value = sscanf( argv [2], "%u", &NUM_TO_MATCH);
		if( (-1 == value) || (MAX_ALLOWED_SEARCH < NUM_TO_MATCH) || (1 > NUM_TO_MATCH)) {
			printf( "Warning: number to match value (%u) is not between 1 and %u\nResetting to 1\n", NUM_TO_MATCH, MAX_ALLOWED_SEARCH);
			NUM_TO_MATCH = 0x01;
		}
	}

	// Allocate space for marking the next bits to check
	const uint64_t NUM_CHECK_SPACES = HASH_SPACE_SIZE / (8 * sizeof( NODE_MAP_LEAF));
	const uint64_t TOTAL_CHECK_SPACE_SIZE = NUM_CHECK_SPACES * sizeof( NODE_MAP_LEAF);
	NODE_MAP_LEAF *pToCheck = (NODE_MAP_LEAF *) calloc( NUM_CHECK_SPACES, sizeof( NODE_MAP_LEAF));
	if( (NODE_MAP_LEAF *) 0x0 == pToCheck) {
		fprintf( stderr, "Failed to allocate %llu bytes for next set marks\n", TOTAL_CHECK_SPACE_SIZE);
		exit( EXIT_FAILURE);
	}
	printf( "Allocated %llu for checking\n", TOTAL_CHECK_SPACE_SIZE);

	// Allocate the container
	CNT_NODES cntNodes;

	// Reserve space in the vector
	const size_t VECTOR_SPACE = MAX_ALLOWED_CHARS * NUM_ALLOWED_CHARS * 2;
	cntNodes.reserve( VECTOR_SPACE);

	// Punch in the first value
	S_WORKING_NODE sNode;
	memset( &sNode, 0x0, sizeof( sNode));
	cntNodes.push_back( sNode);

	// While there is something to do
	while( keepLooping && (0x0 < cntNodes.size())) {

		// Pop for analysis
		const S_WORKING_NODE &curNode = cntNodes.back();
		memcpy( &sNode, &curNode, sizeof( sNode));
		cntNodes.pop_back();
		++ countProcessed;

		// Populate
		for( size_t curChar = 0; NUM_ALLOWED_CHARS > curChar; ++ curChar) {

			// Make the node
			S_WORKING_NODE nextNode;
			memcpy( &nextNode, &sNode, sizeof( sNode));
			nextNode.hashValue *= 31;
			nextNode.hashValue += ALLOWED_CHARS [curChar];
			nextNode.strValue [nextNode.strSize] = ALLOWED_CHARS [curChar];
			++ nextNode.strSize;

			// Matching node?
			if( (HASH_TO_CHECK == nextNode.hashValue) && (0x0 != strcmp( STR_TO_CHECK, nextNode.strValue))) {
				printf( "Found match: %s\n", nextNode.strValue);
				++ numFound;
				keepLooping = (NUM_TO_MATCH > numFound);
			}

			// Attach?
			bool bAttach = nodeIsClear( pToCheck, nextNode.hashValue) && (MAX_ALLOWED_CHARS > nextNode.strSize);
			if( bAttach) {
				cntNodes.push_back( nextNode);
				nodeSet( pToCheck, nextNode.hashValue);
				++ countRecorded;
			}

		} // endfor loop over chars

		// Report?
		if( 0x0 == (countProcessed % 12343113)) {
			printf( "Now at %llu with %llu seen at vector %lu\n", countProcessed, countRecorded, cntNodes.size());
		}

	} // endwhile keep looping

	// Free and done
	free( pToCheck);
	pToCheck = (NODE_MAP_LEAF *) 0x0;
	exit( EXIT_SUCCESS);

}

