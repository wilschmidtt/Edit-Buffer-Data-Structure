#include "EditBuffer.h" 	 	    		
#include <stdlib.h> 
#include <string.h>
#include <stdio.h>

struct STRUCT_EDIT_BUFFER_TAG{
    char* buffer;
    size_t buffer_len;
    int gap_left;
    int gap_right;
};

SEditBufferRef EditBufferCreate(){
    SEditBufferRef ebuf = malloc(sizeof(SEditBuffer));
    ebuf -> buffer_len = 1024;
    ebuf -> buffer = malloc(sizeof(char)*ebuf->buffer_len);
    ebuf -> gap_left = 0;
    ebuf -> gap_right = ebuf->buffer_len - ebuf->gap_left - 1;
    return ebuf;
}

void EditBufferDestroy(SEditBufferRef ebuf){
    free(ebuf->buffer);
    free(ebuf);
}

size_t EditBufferSize(SEditBufferRef ebuf){
    int gap_size = ebuf->gap_right + 1 - ebuf->gap_left;
    return ebuf->buffer_len - gap_size;
}

// grow() will be called when ebuf->gap_size reaches 1
void grow(SEditBufferRef ebuf) {
     int old_buffer_len = ebuf->buffer_len; 
     ebuf->buffer_len = ebuf->buffer_len*2;
     char* temp = malloc(sizeof(char)*ebuf->buffer_len);
     // Copy all values before the cursor
     for(int i=0; i < ebuf->gap_left; i++) {
	     temp[i] = ebuf->buffer[i];
     }
     // Copy all values after the cursor
     int j = 1;
     for(int z=ebuf->gap_left+old_buffer_len; z < ebuf->buffer_len; z++) {
	     temp[z] = ebuf->buffer[ebuf->gap_left + j];
	     j += 1;
     }
     free(ebuf->buffer);
     ebuf->buffer = temp;
}


void left(int position, SEditBufferRef ebuf) {
    // Add error checking in case position is off the edge of buffer
    while(position < ebuf->gap_left && ebuf->gap_left > 0) {
	    ebuf->gap_left -= 1;
	    ebuf->gap_right -= 1;
	    ebuf->buffer[ebuf->gap_right+1] = ebuf->buffer[ebuf->gap_left];
	    ebuf->buffer[ebuf->gap_left]=' ';
    }
}

void right(int position, SEditBufferRef ebuf) {
    // Add error checking in case position is off the edge of buffer
    while(position > ebuf->gap_left && ebuf->gap_right < (ebuf->buffer_len-1)) {
	    ebuf->gap_left += 1;
	    ebuf->gap_right += 1;
	    ebuf->buffer[ebuf->gap_left-1] = ebuf->buffer[ebuf->gap_right];
	    ebuf->buffer[ebuf->gap_right]=' ';
    }
}

size_t EditBufferMoveCursor(SEditBufferRef ebuf, int offset, int origin){
    if(origin==0) {
	    if(offset < ebuf->gap_left) {
		    left(offset, ebuf);
	    }
	    else if(offset > ebuf->gap_left) {
		    right(offset, ebuf);
	    }
    }
    else if(origin==1) {
	    int position = offset + ebuf->gap_left;
	    if(position < ebuf->gap_left) {
		    left(position, ebuf);
	    }
	    else if(position > ebuf->gap_left) {
	   	    right(position, ebuf);
	    }
    }
    else if(origin==2) {
	    int position = ebuf->buffer_len - 1 + offset - (ebuf->gap_right - ebuf->gap_left);
	    if(position < ebuf->gap_left) {
		    left(position, ebuf);
	    }
	    else if(position > ebuf->gap_left) {
		    right(position, ebuf);
	    }
    }
    // Return offset from the beginning of Edit Buffer
    return ebuf->gap_left;
}

size_t EditBufferInsert(SEditBufferRef ebuf, const char *buf, size_t count);

size_t EditBufferLoad(SEditBufferRef ebuf, const char *buf){
    /* 1) Create new, empty edit buffer
     * 2) Load string into beginning of empty edit buffer
     * 3) Move cursor to beginning of new edit buffer
     * 4) Free up space from old edit buffer
     * 5) Update ebuf->buffer to point to the new edit buffer
     */
    int count = strlen(buf);
    char* temp = malloc(sizeof(char)*ebuf->buffer_len);
    free(ebuf->buffer);
    ebuf->buffer = temp;
    EditBufferInsert(ebuf, buf, count);
    left(0, ebuf);
    return EditBufferSize(ebuf);
}

size_t EditBufferRead(SEditBufferRef ebuf, char *buf, size_t count){
    int i=0;
    int characters_read = 0;
    while(i < count) {
	    // Check for a character directly to the right of the cursor
	    if(ebuf->gap_right+1 < ebuf->buffer_len) { 
	    	buf[i] = ebuf->buffer[ebuf->gap_right + 1];
		// Move cursor one position to the right
		right(ebuf->gap_left + 1, ebuf); 
	    	i += 1;
		characters_read += 1;
	    }
	    // No characters exist to the right of the cursor
	    else {
		    return characters_read;
		    break;
	    }
    }
    return characters_read;
}

size_t EditBufferInsert(SEditBufferRef ebuf, const char *buf, size_t count){
    int i = 0;
    int characters_inserted = 0;
    while(i < count) {
	    // Check if gap size equals 1
            if((ebuf->gap_right - ebuf->gap_left) == 1) {
		    grow(ebuf);
	    }
	    // Insert character at ebuf->gap_left then increment gap_left by 1
	    ebuf->buffer[ebuf->gap_left] = buf[i];
	    ebuf->gap_left += 1;
	    i += 1;
	    characters_inserted += 1;
    }
    return characters_inserted;
}

size_t EditBufferDelete(SEditBufferRef ebuf, size_t count){
    int i = 0;
    int characters_deleted = 0;

    while(i < count) {
	    // Check if character exists directly after cursor
	    // If character exists, set char equal to ' ' and grow gap_right by 1
	    if(ebuf->gap_right+1 < ebuf->buffer_len) {
		    ebuf->buffer[ebuf->gap_right+1] = ' ';
		    characters_deleted += 1;
		    i += 1;
		    ebuf->gap_right += 1;
	    }
	    else {
		    return characters_deleted;
		    break;
	    }
    }
    return characters_deleted;
}
