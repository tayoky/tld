#include <stdio.h>
#include "tld.h"

int bin_save(tld_file *file){
	//sections are already sorted for us
	//so easy pz
	
	rewind(file->file);
	for(size_t i=0; i<file->sections_count;i++){
		fwrite(file->sections[i].data,file->sections[i].size,1,file->file);
		if(i < file->sections_count - 1){
			//padding
			for(size_t j=file->sections[i].address+file->sections[i].size; j<file->sections[i+1].address; j++){
				fputc(0,file->file);
			}
		}
	}
	return 0;
}
