#include <iostream>
#include <cstdlib> // Required for atoi

using namespace std;

char* number_to_ascii(char* numerated_message){
    char* encrypted = (char *)  malloc(strlen(numerated_message)*6*sizeof(char)); 
    encrypted[0] = '\0';
    int i = 0;
    while (numerated_message[i]){
        char current_letter = numerated_message[i];
        int ascii_val = int(current_letter);
        if(ascii_val <48 || ascii_val >122 || (ascii_val > 57 && ascii_val < 65) || (ascii_val > 90 && ascii_val < 97) ){
            char current_letter_str[2];
            current_letter_str[0] = current_letter;
            current_letter_str[1] = '\0';
            strcat(encrypted, current_letter_str);
            i++;
            continue;
        }
        char ascii_val_string[100]; 
        snprintf(ascii_val_string, 100, "%d", ascii_val);
        strcat(encrypted, ascii_val_string);
        i++;
    }
    return encrypted;
}

char* ascii_to_num(char* ascii_message){
    char* decrypted = (char *)  malloc(strlen(ascii_message)*6*sizeof(char)); 
    decrypted[0] = '\0';
    int i = 0;
    while (ascii_message[i]){
        char current_letter = ascii_message[i];
        int ascii_val = int(current_letter);
        if(ascii_val <48 || ascii_val >122 || (ascii_val > 57 && ascii_val < 65) || (ascii_val > 90 && ascii_val < 97) ){
            char current_letter_str[2];
            current_letter_str[0] = current_letter;
            current_letter_str[1] = '\0';
            strcat(decrypted, current_letter_str);
            i++;
            continue;
        }

        // the 2 chars together store ascii value of the number
        int current_char = ascii_message[i] - '0';
        int next_char = ascii_message[i+1] - '0';
        int cur_ascii = (current_char)*10 + (next_char);

        // converting ascii value to the number as a character
        char char_to_add = (char) cur_ascii;
        char cur[2];
        cur[0] = char_to_add;
        cur[1] = '\0';
        strcat(decrypted, cur);
        i+=2;
    }
    return decrypted;
}

char* encrypt_message_advanced(char* message){

    char* current_encrypted = (char *)  malloc(strlen(message)*20*sizeof(char)); 
    current_encrypted[0] = '\0';
    int i = 0;

    // Step 1: iterate through message, change to len+ascii of each letter -> new_message
    while (message[i]){
        // Step 1: Determine ASCII value of each letter
        char current_letter = message[i];
        int ascii_val = int(current_letter);

        // check if alphanumeric
        if(ascii_val <48 || ascii_val >122 || (ascii_val > 57 && ascii_val < 65) || (ascii_val > 90 && ascii_val < 97) ){
            char current_letter_str[2];
            current_letter_str[0] = current_letter;
            current_letter_str[1] = '\0';
            strcat(current_encrypted, current_letter_str);
            i++;
            continue;
        }

        // Step 2: Append number of digits to the front
        char ascii_val_string[100]; 
        snprintf(ascii_val_string, 100, "%d", ascii_val);
        
        int len = strlen(ascii_val_string); // Determine if append 2 or 3
        char append_ascii_val_string[100];
        snprintf(append_ascii_val_string, 100, "%d%s",len,ascii_val_string); // Appends the number to the begginging of the string
        strcat(current_encrypted, append_ascii_val_string);
        i++;
    }

    // printf("step 1: %s \n",current_encrypted);
    // Step 2: new_message -> convert each number's char to ascii
    char* encrypted_twice = number_to_ascii(current_encrypted);
    free(current_encrypted);
    // printf("step 2: %s \n",encrypted_twice);

    // Step 3 (repeat): new_message -> convert each number's char to ascii
    char* encrypted_thrice = number_to_ascii(encrypted_twice);
    // printf("step 3: %s \n",encrypted_thrice);
    free(encrypted_twice);
    
    // Final Step -> convert to lowercase, uppercase or numbers based on even odd
    int cur_num = 0;
    bool cur_even = true;
    int j = 0;
    int pos = 0;
    char* final_encrypted = (char *) malloc(strlen(message)*20*sizeof(char)); 
    final_encrypted[0] = '\0';
    while(encrypted_thrice[j]){
        char current_letter = encrypted_thrice[j];
        int ascii_val = int(current_letter);

        if(ascii_val <48 || ascii_val >122 || (ascii_val > 57 && ascii_val < 65) || (ascii_val > 90 && ascii_val < 97) ){
            char current_letter_str[2];
            current_letter_str[0] = current_letter;
            current_letter_str[1] = '\0';
            strcat(final_encrypted, current_letter_str);
            j++;
            pos = 0;
            continue;
        }
        int current_char = encrypted_thrice[j] - '0';
        int next_char = encrypted_thrice[j+1] - '0';
        cur_num = (current_char)*10 + (next_char);
        char character_to_add = ' ';
        // Case1: number is even : keep number add 1, set cur_eve true
        if(cur_num % 2 == 0){
            cur_even = true;
            int final_ascii_val = cur_num + 1;
            character_to_add = (char)final_ascii_val;
        }
        // Case2: number is odd, but cur_even: Capital number, set cur_eve false
        else if(cur_num % 2 == 1 && cur_even){
            cur_even = false;
            int final_ascii_val = cur_num + 17 + (pos * 2);
            character_to_add = (char)final_ascii_val;
        }
        // Case3: number is odd, cur_even is false: lowercase number, set cur_eve false
        else if(cur_num % 2 == 1 && !cur_even){
            int final_ascii_val = cur_num + 49 + (pos * 2);
            character_to_add = (char)final_ascii_val;
        }

        if(pos == 8){
            pos = 0;
        }else{
            pos++;
        }
        j+=2;

        char char_to_concat[2];
        char_to_concat[0] = character_to_add;
        char_to_concat[1] = '\0';
        strcat(final_encrypted, char_to_concat);
    }

    // printf("step 4: %s \n",final_encrypted);
    free(encrypted_thrice);
    return final_encrypted;
}

char* decrypt_message_advanced(char* encrypted){
    char* decrypted = (char *) malloc(strlen(encrypted)*20*sizeof(char)); 
    decrypted[0] = '\0';
    // int cur_num = 0;
    // bool cur_even = true;
    int j = 0;
    int pos = 0;
    char* cur_decrypted = (char *) malloc(strlen(encrypted)*20*sizeof(char)); 
    cur_decrypted[0] = '\0';
    while(encrypted[j]){
        char current_letter = encrypted[j];
        int ascii_val = int(current_letter);

        char decoded[100]; 
        // Case1: character is a number 
        if(ascii_val > 47 && ascii_val <58){
            int decoded_num = ascii_val -1;
            snprintf(decoded, 100, "%d", decoded_num);
        }
        // Case2: Capital letter
        else if(ascii_val > 64 && ascii_val <91){
            int decoded_num = ascii_val - 17 - (pos * 2);
            snprintf(decoded, 100, "%d", decoded_num);
        }
        // Case3: Lowercase letter
        else if(ascii_val > 96 && ascii_val <123){
            int decoded_num = ascii_val - 49 - (pos * 2);
            snprintf(decoded, 100, "%d", decoded_num);
        }
        else {
            char current_letter_str[2];
            current_letter_str[0] = current_letter;
            current_letter_str[1] = '\0';
            strcat(cur_decrypted, current_letter_str);
            j++;
            pos = 0;
            continue;
        }

        strcat(cur_decrypted, decoded);        
        if(pos == 8){
            pos = 0;
        }else{
            pos++;
        }
        j++;
    }

    // printf("decode 1: %s\n", cur_decrypted);
    char* decrypted_twice = ascii_to_num(cur_decrypted);
    free(cur_decrypted);

    // printf("decode 2: %s\n", decrypted_twice);
    char* decrypted_thrice = ascii_to_num(decrypted_twice);
    free(decrypted_twice);
    // printf("decode 3: %s\n", decrypted_thrice);

    int i = 0;
    while(decrypted_thrice[i]){
        char current_letter = decrypted_thrice[i];
        int ascii_val = int(current_letter);
        if(ascii_val <48 || ascii_val >122 || (ascii_val > 57 && ascii_val < 65) || (ascii_val > 90 && ascii_val < 97) ){
            char current_letter_str[2];
            current_letter_str[0] = current_letter;
            current_letter_str[1] = '\0';
            strcat(decrypted, current_letter_str);
            i++;
            continue;
        }

        // ascii value of '2', next 2 digits are ascii
        if(ascii_val == 50){
            // convert it to ascii
            int tens_num = decrypted_thrice[i+1] - '0';
            int ones_num = decrypted_thrice[i+2] - '0';
            int final_asci_decode = (tens_num)*10 + (ones_num);
            char current_letter = (char)final_asci_decode;
            // add it to decrypted string
            char current_letter_str[2];
            current_letter_str[0] = current_letter;
            current_letter_str[1] = '\0';
            strcat(decrypted, current_letter_str);
            i+=3;
        }else if(ascii_val == 51){
            // convert it to ascii
            int hund_num = decrypted_thrice[i+1] - '0';
            int tens_num = decrypted_thrice[i+2] - '0';
            int ones_num = decrypted_thrice[i+3] - '0';

            int final_asci_decode = hund_num*100+ (tens_num)*10 + (ones_num);
            char current_letter = (char)final_asci_decode;
            // add it to decrypted string
            char current_letter_str[2];
            current_letter_str[0] = current_letter;
            current_letter_str[1] = '\0';
            strcat(decrypted, current_letter_str);
            i+=4;
        }else{
            i++;
        }
    }

    return decrypted;
}


int main(int argc, char** argv){
    // Usage: ./advanced_encryption encrypt/decrypt string-to-encrypt-or-decrypt

    if(argc < 3){
        printf("Incorrect usage, provide these 2 arguments: (encrypt/decrypt) (string-to-encrypt-or-decrypt)");
    }

    if (strcmp(argv[1], "encrypt") == 0){
        for(int i = 2; i<argc; i++){
            char*encrypted = encrypt_message_advanced(argv[i]);
            printf("%s ", encrypted);
        }
    }

    else if (strcmp(argv[1], "decrypt") == 0){
        for(int i = 2; i<argc; i++){
            char*decrypted = decrypt_message_advanced(argv[i]);
            printf("%s ", decrypted);
        }
    }

    return 0; 
}