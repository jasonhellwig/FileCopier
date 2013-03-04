#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdlib>
#include <unistd.h>
#include <sys/mman.h>
#include <cstring>
#include <cstdio>

using namespace std;

int main(int argc, char* argv[])
{
	//check command line arguments
	if(argc != 3)
	{
		cerr<<"invalid number of arguments.\nUSAGE: ./copier [source file] [destination file]\n";
		exit(-1);
	}
	
	//open the source file
	int fd_source = open(argv[1],O_RDONLY);
	
	//get sourcefile size
	struct stat source_stat;
	fstat(fd_source,&source_stat);	
	int length = source_stat.st_size;
	
	//open destination file to set length
	FILE* dest_file = fopen(argv[2],"w+");
	fseek(dest_file,length-1,SEEK_SET);
	
	//write a single byte to mark the end point for mmap
	fwrite("",sizeof(""),1,dest_file);
	
	//close the file
	fclose(dest_file);
	
	//reopen desitination file with open to get a file descriptor
	int fd_dest = open(argv[2],O_RDWR);
	
	//check if files opened correctly
	if (fd_source<0 || fd_dest<0)
	{
		cerr<<"open\n";
		exit(-1);
	}
	
	//get page size
	int pageSize = getpagesize();	
	
	
	//copy up to one pageSize at a time
	//initial conditions	
	size_t mapSize = 0;
	off_t offset = 0;
	
	//start mapping
	while(length > 0)
	{
		//figure out how much to map in this iteration
		if(length > pageSize)
			mapSize = pageSize;
		else mapSize = length;
		
		//map files into memory
		char* source = (char*)mmap(NULL,mapSize,PROT_READ,MAP_SHARED,fd_source,offset);
		if(!source)
		{
			cerr<<"source mmap\n";
			exit(-1);
		}		
		char* dest = (char*)mmap(NULL,mapSize,PROT_READ|PROT_WRITE,MAP_SHARED,fd_dest,offset);
		if(!dest)
		{
			cerr<<"dest mmap\n";
			exit(-1);
		}
		
		//copy the mapped area from the source to the destination
		memcpy(dest,source,mapSize);
		
		//update iterators
		offset += mapSize;
		length -= mapSize;	
		
		//unmap memory
		munmap((void*)source,mapSize);
		munmap((void*)dest,mapSize);
		
		
	}	
	
	//close files
	close(fd_source);
	close(fd_dest);
	
	return 0;
}


