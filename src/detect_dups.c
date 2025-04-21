#include "detect_dups.h"

static PathList *HardLinks = NULL;
static PathList *SoftLinks = NULL;
static PathList *Duplicates = NULL;

static void print_md5_hash(uint8 *MD5Hash) {
    printf("\tMD5 Hash: ");
    for(int i = 0; i < MD5_DIGEST_LENGTH; i++) {
	printf("%02x", MD5Hash[i]);
    }
    printf("\n");
}

// NOTE: This really doesn't have any file allocation.

void n_print_files() {
    int FileCount = 1; 
    for(PathList *dups = Duplicates; dups != NULL; dups = dups->hh.next) {
	printf("File %d\n", FileCount++);
	print_md5_hash(dups->HashKey);

	// iterate through each duplicate 
	for(int i = 0; i < dups->Size; i++) {
	    struct stat StatBuffer;
	    stat(dups->Paths[i], &StatBuffer);

	    // get the soft links and hard links attached to that duplicate
	    PathList *FileHardLinks, *FileSoftLinks;
	    HASH_FIND_INT(HardLinks, &(StatBuffer.st_ino), FileHardLinks);
	    HASH_FIND_INT(SoftLinks, &(StatBuffer.st_ino), FileSoftLinks);

	    if (FileHardLinks) {
		printf("\t\tHard Link (%d): %ld\n", FileHardLinks->Size, StatBuffer.st_ino);
		printf("\t\t\tPaths:\t%s\n", FileHardLinks->Paths[0]);
		for(int j = 1; j < FileHardLinks->Size; j++) {
		    printf("\t\t\t\t%s\n", FileHardLinks->Paths[j]);
		}
	    }

	    if(FileSoftLinks) {
		int SoftLinkCount = 1;
		for(int j = 0; j < FileSoftLinks->Size; j++) {
		    struct stat SoftLinkStatBuffer;
		    lstat(FileSoftLinks->Paths[j], &SoftLinkStatBuffer);
		    
		    PathList *FileSoftHardLinks; 
		    HASH_FIND_INT(HardLinks, &(SoftLinkStatBuffer.st_ino), FileSoftHardLinks);

		    if(FileSoftHardLinks) {

			printf("\t\t\tSoft Link %d(%d): %ld\n",
			       SoftLinkCount++,
			       FileSoftHardLinks->Size,
			       SoftLinkStatBuffer.st_ino);
			printf("\t\t\t\tPaths:\t%s\n", FileSoftHardLinks->Paths[0]);
			for(int k = 1; k < FileSoftHardLinks->Size; k++) {
			    printf("\t\t\t\t\t%s\n", FileSoftHardLinks->Paths[k]);
			}
		    }
		}
	    }
	}
    } 
}

static void compute_digest(const char *FilePath, uint8 *Digest) {
    FILE *FileHandle = fopen(FilePath, "r");
    if(!FileHandle) {
	fprintf(stderr, "Couldn't open file\n");
    }

    EVP_MD_CTX *MDContext = EVP_MD_CTX_new();
    EVP_DigestInit_ex(MDContext, EVP_md5(), NULL);

    uint8 Buffer[CHUNK_SIZE];
    int BytesRead; 
    while((BytesRead = fread(Buffer, 1, CHUNK_SIZE, FileHandle)) > 0) {
	EVP_DigestUpdate(MDContext, Buffer, BytesRead); 
    }

    if(EVP_DigestFinal_ex(MDContext, Digest, NULL) <= 0) {
	fprintf(stderr, "Couldn't finalize digest\n");
    } 

    fclose(FileHandle);
}

static PathList *create_pathlist(int INodeValue, const char *FilePath, uint8 *Digest) {
    PathList *Result = malloc(sizeof(PathList));
    Result->Paths = malloc(sizeof(char *) * NO_STRS);
    Result->HashKey = malloc(MD5_DIGEST_LENGTH); 
    Result->INodeValue = INodeValue;
    Result->Size = 0;
	    
    for(int i = 0; i < NO_STRS; i++) {
	Result->Paths[i] = malloc(sizeof(char) * MAX_FILE_LENGTH); 
    }

    strcpy((char *) Result->Paths[Result->Size], FilePath);
    if(Digest) 
	memcpy(Result->HashKey, Digest, MD5_DIGEST_LENGTH);

    Result->Size++; 

    return Result;
}

static PathList *free_pathlist(PathList *List) {
    while(List != NULL) {
	PathList *Next = List->hh.next;
	
	free(List->HashKey);
	for(int i = 0; i < List->Size; i++) {
	    free((char *)List->Paths[i]);
	}
	free(List->Paths);
	free(List);

	List = Next;
    }
}

static void append_to_file_list(PathList *List, const char *FilePath) {
    strcpy((char *) List->Paths[List->Size], FilePath);
    List->Size++;
}
static int render_file_info(const char *FilePath, const struct stat *StatBuffer,
			    int TFlag, struct FTW *FTWBuffer){
    if(TFlag == FTW_F) {
	PathList *HardLinkList, *DuplicateList;
	HASH_FIND_INT(HardLinks, &(StatBuffer->st_ino), HardLinkList);
		
	if(!HardLinkList) {
	    uint8 Digest[MD5_DIGEST_LENGTH];
	    compute_digest(FilePath, Digest);
	    
	    PathList *NewList = create_pathlist(StatBuffer->st_ino, FilePath, Digest);
	    HASH_ADD_INT(HardLinks, INodeValue, NewList);

	    HASH_FIND(hh, Duplicates, NewList->HashKey, MD5_DIGEST_LENGTH, DuplicateList);
	    if(!DuplicateList) {
		PathList *DupList = create_pathlist(StatBuffer->st_ino, FilePath, NewList->HashKey);
		HASH_ADD_KEYPTR(hh, Duplicates, DupList->HashKey, MD5_DIGEST_LENGTH, DupList);
	    } else {
		append_to_file_list(DuplicateList, FilePath);
	    } 
	} else {
	    append_to_file_list(HardLinkList, FilePath);
	}
	
    } else if(TFlag == FTW_SL) {
	PathList *HardLinkList; 
	HASH_FIND_INT(HardLinks, &(StatBuffer->st_ino), HardLinkList);
	if(!HardLinkList) {
	    PathList *NewHardLinkList = create_pathlist(StatBuffer->st_ino, FilePath, NULL);
	    HASH_ADD_INT(HardLinks, INodeValue, NewHardLinkList);

	    struct stat SourceStat;
	    stat(FilePath, &SourceStat);
	    PathList *SoftLinkList;
	    HASH_FIND_INT(SoftLinks, &(SourceStat.st_ino), SoftLinkList);

	    if(!SoftLinkList) {
		PathList *NewList = create_pathlist(SourceStat.st_ino, FilePath, NULL);
		HASH_ADD_INT(SoftLinks, INodeValue, NewList);
	    } else {
		append_to_file_list(SoftLinkList, FilePath);
	    }

	} else {
	    append_to_file_list(HardLinkList, FilePath);

	}
    }

    return 0;
}

int main(int argc, char **argv) {
    if(argc == 2) {
	clock_t start = clock(); 
	nftw(argv[1], render_file_info, 50, FTW_PHYS);
	n_print_files();

	clock_t diff = clock() - start;
	double ms = ((double) diff / (double) CLOCKS_PER_SEC) * 1000;
	printf("Total Time Taken: %.2lfms\n", ms);

	// NOTE(Rainy): It's better to not free memory and let the OS handle that. 
//	free_pathlist(HardLinks);
//	free_pathlist(SoftLinks);
//	free_pathlist(Duplicates);
	
	
    } else {
	fprintf(stderr, "Usage: ./detect_dups <directory_name>\n");
    }
    
    return 0; 
}
