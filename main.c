/*      Twitter
 *     (Client)
 *  By Ali Razghandi
 *
 *                    */

/*importing libraries*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>
#include <unistd.h>
#include "cJSON.h"

// functions prototypes

char *signup(char buffer[10000]);

char *login(char buffer[10000]);

char *send_tweet(char buffer[10000]);

char *logout(char buffer[10000]);

char *change(char buffer[10000]);

char *set(char buffer[10000]);

char *profile(char buffer[10000]);

char *search(char buffer[10000]);

char *follow(char buffer[10000]);

char *unfollow(char buffer[10000]);

char *refresh(char buffer[10000]);

char *like(char buffer[10000]);

char *comment(char buffer[10000]);

char *delete(char buffer[10000]);

// Server Parts

cJSON *online_data;

struct sockaddr_in server_addr;

int initialize () {
    WSADATA wsadata;
    int wsastartup = WSAStartup(MAKEWORD(2, 2), &wsadata);
    if (wsastartup != 0) {
        printf("Initialize failed\n");
        return INVALID_SOCKET;
    }else
    {
        printf("Initialized\n");
    }
    int port = 12345;

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket == INVALID_SOCKET) {
        printf("Invalid socket\n");
        return INVALID_SOCKET;
    }else{
        printf("Socket created\n");
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    int bind_res = bind(server_socket,(struct sockaddr*) &server_addr, sizeof(server_addr));

    if (bind_res == SOCKET_ERROR){
        printf("Bind failed\n");
        return INVALID_SOCKET;
    }else {
        printf("Bind successfully\n");
    }

    listen(server_socket,SOMAXCONN);

    return server_socket;
}


void start(int server_socket){

    int len = sizeof(struct sockaddr_in);

    int client_socket;
    struct  sockaddr_in client_addr;

    while ((client_socket = accept(server_socket,(struct sockaddr*) &client_addr, &len))!= INVALID_SOCKET){

        printf("Client connected\n");
        char buffer[10000];
        char *response = (char *)calloc(10000,1);
        memset(buffer,0,10000);

        recv(client_socket,buffer,9999,0);

        char input_command[100];
        char command_data[10000];
        memset(input_command,0,100);
        memset(command_data,0,10000);

        sscanf(buffer,"%s",input_command);

        if (!strcmp(input_command,"signup")){

           response = signup(buffer);

        }else if (!strcmp(input_command,"login")){

          response = login(buffer);

        }else if (!strcmp(input_command,"send")){

          response = send_tweet(buffer);

        }else if (!strcmp(input_command,"refresh")){

           response = refresh(buffer);

        }else if (!strcmp(input_command,"like")){

           response = like(buffer);

        }else if (!strcmp(input_command,"comment")){

           response = comment(buffer);

        }else if (!strcmp(input_command,"search")){

          response = search(buffer);

        }else if (!strcmp(input_command,"follow")){

          response = follow(buffer);

        }else if (!strcmp(input_command,"unfollow")){

            response = unfollow(buffer);

        }else if (!strcmp(input_command,"set")){

           response = set(buffer);

        }else if (!strcmp(input_command,"logout")){

            response = logout(buffer);

        }else if (!strcmp(input_command,"profile")){

           response = profile(buffer);

        }else if (!strcmp(input_command,"change")){

            response = change(buffer);

        }else if (!strcmp(input_command,"delete")){

           response = delete(buffer);

        }else{

            sprintf(response,"{\"Error\":\"Bad request\"}\n");

        }

        send(client_socket,response,(int )strlen(response),0);
        closesocket(client_socket);

        free(response);
    }

}

// delete a tweet

char *delete(char buffer[10000]) {
    char *response = (char *)calloc(10000,1);
    char token[100];
    char delete_tweet[100];
    memset(token,0,100);
    memset(delete_tweet,0,100);

    sscanf(buffer,"%*s%s%s",token,delete_tweet);

    token[strlen(token)-1] = '\0';

    char user[100];
    memset(user,0,100);

    cJSON *online = cJSON_GetObjectItemCaseSensitive(online_data,"online");
    cJSON *element;

    for (int i = 0; i < cJSON_GetArraySize(online); ++i) {
        element = cJSON_GetArrayItem(online,i);
        if (!strcmp(token,element->valuestring)){
            strcpy(user,element->string);
            break;
        }
    }
    if (!strlen(user)){
        sprintf(response,"{\"type\":\"Error\",\"message\":\"Token is invalid.\"}");
        printf("{\"type\":\"Error\",\"message\":\"Token is invalid.\"}\n");
        return response;
    }

    char check[110];
    memset(check, 0, 110);
    sprintf(check, "../Resources/Tweets/%s.txt", delete_tweet);

    if (access(check,F_OK)){
        sprintf(response,"{\"type\":\"Error\",\"message\":\"Invalid tweet ID.\"}");
        printf("{\"type\":\"Error\",\"message\":\"Invalid tweet ID.\"}\n");
        return response;
    }

    char save[10000];
    memset(save,0,10000);

    memset(check, 0, 110);
    sprintf(check, "../Resources/Users/%s.txt", user);

    FILE *file_pointer;
    file_pointer = fopen(check,"r");
    fgets(save,9999,file_pointer);
    fclose(file_pointer);

    cJSON *user_json = cJSON_Parse(save);
    cJSON *personal_tweets = cJSON_GetObjectItemCaseSensitive(user_json,"personalTweets");
    cJSON *members;
    int IsOwned = 0;

    for (int i = 0; i < cJSON_GetArraySize(personal_tweets); ++i) {
        members = cJSON_GetArrayItem(personal_tweets,i);
        if (atoi(delete_tweet)==members->valueint){
            cJSON_DeleteItemFromArray(personal_tweets,i);
            IsOwned = 1;
            break;
        }
    }

    if (IsOwned == 0){
        sprintf(response,"{\"type\":\"Error\",\"message\":\"This tweet does not belong to you.\"}");
        printf("{\"type\":\"Error\",\"message\":\"This tweet does not belong to you.\"}\n");
        return response;
    }

    file_pointer = fopen(check,"w");
    fprintf(file_pointer,"%s",cJSON_PrintUnformatted(user_json));
    fclose(file_pointer);
    cJSON_Delete(user_json);


    memset(check, 0, 110);
    sprintf(check, "../Resources/Tweets/%s.txt", delete_tweet);

    int IsDeleted;

    IsDeleted = remove(check);

    if (IsDeleted != 0){
        sprintf(response,"{\"type\":\"Error\",\"message\":\"Server Cannot delete this tweet.\"}");
        printf("{\"type\":\"Successful\",\"message\":\"Server Cannot delete this tweet.\"}\n");
        return response;
    }

    sprintf(response,"{\"type\":\"Successful\",\"message\":\"You delete this tweet.\"}");
    printf("{\"type\":\"Successful\",\"message\":\"You delete this tweet.\"}\n");
    return response;
}

// comment for a tweet

char *comment(char buffer[10000]) {
    char *response = (char *)calloc(10000,1);
    char token[100];
    char tweet_id[100];
    char comment_content[1000];
    memset(token,0,100);
    memset(tweet_id,0,100);
    memset(comment_content,0,1000);

    sscanf(buffer,"%*s%s%s%*c%[^\n]s",token,tweet_id,comment_content);

    token[strlen(token)-1] = '\0';
    tweet_id[strlen(tweet_id)-1] = '\0';

    char user[100];
    memset(user,0,100);

    cJSON *online = cJSON_GetObjectItemCaseSensitive(online_data,"online");
    cJSON *element;

    for (int i = 0; i < cJSON_GetArraySize(online); ++i) {
        element = cJSON_GetArrayItem(online,i);
        if (!strcmp(token,element->valuestring)){
            strcpy(user,element->string);
            break;
        }
    }
    if (!strlen(user)){
        sprintf(response,"{\"type\":\"Error\",\"message\":\"Token is invalid.\"}");
        printf("{\"type\":\"Error\",\"message\":\"Token is invalid.\"}\n");
        return response;
    }

    char check[110];
    memset(check, 0, 110);
    sprintf(check, "../Resources/Tweets/%s.txt", tweet_id);

    if (access(check,F_OK)){
        sprintf(response,"{\"type\":\"Error\",\"message\":\"Invalid tweet ID.\"}");
        printf("{\"type\":\"Error\",\"message\":\"Invalid tweet ID.\"}\n");
        return response;
    }

    char save[10000];
    memset(save,0,10000);

    FILE *file_pointer;
    file_pointer = fopen(check,"r");
    fgets(save,9999,file_pointer);
    fclose(file_pointer);

    cJSON *tweet = cJSON_Parse(save);
    cJSON *comments = cJSON_GetObjectItemCaseSensitive(tweet,"comments");
    cJSON_AddStringToObject(comments,user,comment_content);

    file_pointer = fopen(check,"w");
    fprintf(file_pointer,"%s",cJSON_PrintUnformatted(tweet));
    fclose(file_pointer);
    cJSON_Delete(tweet);

    sprintf(response,"{\"type\":\"Successful\",\"message\":\"Your comment has been received.\"}");
    printf("{\"type\":\"Successful\",\"message\":\"Your comment has been received.\"}\n");

    return response;
}

// like a tweet

char *like(char buffer[10000]) {
    char *response = (char *)calloc(10000,1);
    char token[100];
    char liked_tweet[100];
    memset(token,0,100);
    memset(liked_tweet,0,100);

    sscanf(buffer,"%*s%s%s",token,liked_tweet);

    token[strlen(token)-1] = '\0';

    char user[100];
    memset(user,0,100);

    cJSON *online = cJSON_GetObjectItemCaseSensitive(online_data,"online");
    cJSON *element;

    for (int i = 0; i < cJSON_GetArraySize(online); ++i) {
        element = cJSON_GetArrayItem(online,i);
        if (!strcmp(token,element->valuestring)){
            strcpy(user,element->string);
            break;
        }
    }
    if (!strlen(user)){
        sprintf(response,"{\"type\":\"Error\",\"message\":\"Token is invalid.\"}");
        printf("{\"type\":\"Error\",\"message\":\"Token is invalid.\"}\n");
        return response;
    }

    char check[110];
    memset(check, 0, 110);
    sprintf(check, "../Resources/Tweets/%s.txt", liked_tweet);

    if (access(check,F_OK)){
        sprintf(response,"{\"type\":\"Error\",\"message\":\"Invalid tweet ID.\"}");
        printf("{\"type\":\"Error\",\"message\":\"Invalid tweet ID.\"}\n");
        return response;
    }

    char save[10000];
    memset(save,0,10000);

    memset(check, 0, 110);
    sprintf(check, "../Resources/Users/%s.txt", user);

    FILE *file_pointer;
    file_pointer = fopen(check,"r");
    fgets(save,9999,file_pointer);
    fclose(file_pointer);

    cJSON *user_json = cJSON_Parse(save);
    cJSON *liked_json = cJSON_GetObjectItemCaseSensitive(user_json,"likedTweets");
    cJSON *members;
    int isliked = 0;

    for (int i = 0; i < cJSON_GetArraySize(liked_json); ++i) {
        members = cJSON_GetArrayItem(liked_json,i);
        if (atoi(liked_tweet)==members->valueint){
            isliked = 1;
            break;
        }
    }

    if (isliked == 1){
        sprintf(response,"{\"type\":\"Error\",\"message\":\"You already liked this tweet.\"}");
        printf("{\"type\":\"Error\",\"message\":\"You already liked this tweet.\"}\n");
        return response;
    }
    cJSON_AddItemToArray(liked_json,cJSON_CreateNumber(atoi(liked_tweet)));

    file_pointer = fopen(check,"w");
    fprintf(file_pointer,"%s",cJSON_PrintUnformatted(user_json));
    fclose(file_pointer);
    cJSON_Delete(user_json);


    memset(save,0,10000);
    memset(check, 0, 110);
    sprintf(check, "../Resources/Tweets/%s.txt", liked_tweet);

    file_pointer = fopen(check,"r");
    fgets(save,9999,file_pointer);
    fclose(file_pointer);

    cJSON *tweet = cJSON_Parse(save);
    cJSON *likes = cJSON_GetObjectItemCaseSensitive(tweet,"likes");
    cJSON_ReplaceItemInObjectCaseSensitive(tweet,"likes",cJSON_CreateNumber((likes->valueint)+1));

    file_pointer = fopen(check,"w");
    fprintf(file_pointer,"%s",cJSON_PrintUnformatted(tweet));
    fclose(file_pointer);
    cJSON_Delete(tweet);

    sprintf(response,"{\"type\":\"Successful\",\"message\":\"You liked this tweet.\"}");
    printf("{\"type\":\"Successful\",\"message\":\"You liked this tweet.\"}\n");

    return response;
}

// refresh function

char *refresh(char buffer[10000]) {
    char *response = (char *)calloc(10000,1);
    char token[100];
    memset(token,0,100);

    sscanf(buffer,"%*s%s",token);

    char user[100];
    memset(user,0,100);

    cJSON *online = cJSON_GetObjectItemCaseSensitive(online_data,"online");
    cJSON *element;

    for (int i = 0; i < cJSON_GetArraySize(online); ++i) {
        element = cJSON_GetArrayItem(online,i);
        if (!strcmp(token,element->valuestring)){
            strcpy(user,element->string);
            break;
        }
    }
    if (!strlen(user)){
        sprintf(response,"{\"type\":\"Error\",\"message\":\"Token is invalid.\"}");
        printf("{\"type\":\"Error\",\"message\":\"Token is invalid.\"}\n");
        return response;
    }

    char check[110];
    memset(check, 0, 110);
    sprintf(check, "../Resources/Users/%s.txt", user);

    char save[10000];
    memset(save,0,10000);

    FILE *file_pointer;
    file_pointer = fopen(check,"r");
    fgets(save,9999,file_pointer);
    fclose(file_pointer);

    cJSON *user_json = cJSON_Parse(save);
    cJSON *followers = cJSON_GetObjectItemCaseSensitive(user_json,"followers");
    cJSON *followings = cJSON_GetObjectItemCaseSensitive(user_json,"followings");
    cJSON *seen_tweet = cJSON_GetObjectItemCaseSensitive(user_json,"seenTweets");
    cJSON *show_tweets = cJSON_CreateArray();
    cJSON *author;
    cJSON *author_tweets;
    cJSON *parser;
    int isseen;

    FILE *file_pointer_tweet;
    char check_tweet[110];
    char save_tweet[10000];
    cJSON *parser_tweet;

    for (int i = 0; i < cJSON_GetArraySize(followers); ++i) {
        author = cJSON_GetArrayItem(followers,i);
        memset(check, 0, 110);
        sprintf(check, "../Resources/Users/%s.txt", author->valuestring);
        memset(save,0,10000);
        file_pointer = fopen(check,"r");
        fgets(save,9999,file_pointer);
        fclose(file_pointer);
        parser = cJSON_Parse(save);
        author_tweets = cJSON_GetObjectItemCaseSensitive(parser,"personalTweets");
        for (int j = 0; j < cJSON_GetArraySize(author_tweets); ++j) {
            isseen = 0;
            for (int k = 0; k < cJSON_GetArraySize(seen_tweet); ++k) {
                if (cJSON_GetArrayItem(seen_tweet,k)->valueint == cJSON_GetArrayItem(author_tweets,j)->valueint){  /* check the tweet is seen or not before */
                    isseen = 1;
                    break;
                }
            }
            if (isseen == 0){

                /* get the unseen tweet*/
                memset(check_tweet, 0, 110);
                sprintf(check_tweet, "../Resources/Tweets/%d.txt", cJSON_GetArrayItem(author_tweets,j)->valueint);
                memset(save_tweet,0,10000);

                file_pointer_tweet = fopen(check_tweet,"r");
                fgets(save_tweet,9999,file_pointer_tweet);
                fclose(file_pointer_tweet);
                parser_tweet = cJSON_Parse(save_tweet);
                cJSON_AddItemToArray(show_tweets,parser_tweet);

                /* mark the tweet as a seen tweet in the users file */
                cJSON_AddItemToArray(seen_tweet,cJSON_CreateNumber(cJSON_GetArrayItem(author_tweets,j)->valueint));
            }
        }
    }
    for (int i = 0; i < cJSON_GetArraySize(followings); ++i) {
        author = cJSON_GetArrayItem(followings,i);
        memset(check, 0, 110);
        sprintf(check, "../Resources/Users/%s.txt", author->valuestring);
        memset(save,0,10000);
        file_pointer = fopen(check,"r");
        fgets(save,9999,file_pointer);
        fclose(file_pointer);
        parser = cJSON_Parse(save);
        author_tweets = cJSON_GetObjectItemCaseSensitive(parser,"personalTweets");
        for (int j = 0; j < cJSON_GetArraySize(author_tweets); ++j) {
            isseen = 0;
            for (int k = 0; k < cJSON_GetArraySize(seen_tweet); ++k) {
                if (cJSON_GetArrayItem(seen_tweet,k)->valueint == cJSON_GetArrayItem(author_tweets,j)->valueint){  /* check the tweet is seen or not before */
                    isseen = 1;
                    break;
                }
            }
            if (isseen == 0){

                /* get the unseen tweet*/
                memset(check_tweet, 0, 110);
                sprintf(check_tweet, "../Resources/Tweets/%d.txt", cJSON_GetArrayItem(author_tweets,j)->valueint);
                memset(save_tweet,0,10000);

                file_pointer_tweet = fopen(check_tweet,"r");
                fgets(save_tweet,9999,file_pointer_tweet);
                fclose(file_pointer_tweet);
                parser_tweet = cJSON_Parse(save_tweet);
                cJSON_AddItemToArray(show_tweets,parser_tweet);

                /* mark the tweet as a seen tweet in the users file */
                cJSON_AddItemToArray(seen_tweet,cJSON_CreateNumber(cJSON_GetArrayItem(author_tweets,j)->valueint));
            }
        }
    }
    memset(check, 0, 110);
    sprintf(check, "../Resources/Users/%s.txt", user);
    file_pointer = fopen(check,"w");
    fprintf(file_pointer,"%s",cJSON_PrintUnformatted(user_json));
    fclose(file_pointer);
    cJSON_Delete(user_json);

    cJSON *response_json = cJSON_CreateObject();
    cJSON_AddStringToObject(response_json,"type","List");
    cJSON_AddItemToObject(response_json,"message",show_tweets);

    char *response_now = (char *)calloc(strlen(cJSON_PrintUnformatted(response_json)) + 100,1);

    sprintf(response_now,"%s",cJSON_PrintUnformatted(response_json));
    printf("%s",cJSON_PrintUnformatted(response_json));

    cJSON_Delete(response_json);
    return response_now;
}

// unfollow an account

char *unfollow(char buffer[10000]) {

    char *response = (char *)calloc(10000,1);
    char token[100];
    char search[100];
    memset(token,0,100);
    memset(search,0,100);

    sscanf(buffer,"%*s%s%s",token,search);

    token[strlen(token)-1] = '\0';

    char user[100];
    memset(user,0,100);

    cJSON *online = cJSON_GetObjectItemCaseSensitive(online_data,"online");
    cJSON *element;

    for (int i = 0; i < cJSON_GetArraySize(online); ++i) {
        element = cJSON_GetArrayItem(online,i);
        if (!strcmp(token,element->valuestring)){
            strcpy(user,element->string);
            break;
        }
    }
    if (!strlen(user)){
        sprintf(response,"{\"type\":\"Error\",\"message\":\"Token is invalid.\"}");
        printf("{\"type\":\"Error\",\"message\":\"Token is invalid.\"}\n");
        return response;
    }

    char check[110];
    memset(check, 0, 110);
    sprintf(check, "../Resources/Users/%s.txt", search);

    if (access(check,F_OK)){
        sprintf(response,"{\"type\":\"Error\",\"message\":\"This username is not valid.\"}");
        printf("{\"type\":\"Error\",\"message\":\"This username is not valid.\"}\n");
        return response;
    }

    char save[10000];
    memset(save,0,10000);

    FILE *file_pointer;
    file_pointer = fopen(check,"r");
    fgets(save,9999,file_pointer);
    fclose(file_pointer);

    cJSON *user_json = cJSON_Parse(save);
    cJSON *followers = cJSON_GetObjectItemCaseSensitive(user_json,"followers");
    cJSON *members;
    int isfollow = 0;

    for (int i = 0; i < cJSON_GetArraySize(followers); ++i) {
        members = cJSON_GetArrayItem(followers,i);
        if (strcmp(members->valuestring,user) == 0){
            cJSON_DeleteItemFromArray(followers,i);
            isfollow = 1;
            break;
        }
    }

    if (isfollow == 0){
        sprintf(response,"{\"type\":\"Error\",\"message\":\"You are not following this account.\"}");
        printf("{\"type\":\"Error\",\"message\":\"You are not following this account.\"}\n");
        return response;
    }

    file_pointer = fopen(check,"w");
    fprintf(file_pointer,"%s",cJSON_PrintUnformatted(user_json));
    fclose(file_pointer);
    cJSON_Delete(user_json);

    memset(check, 0, 110);
    sprintf(check, "../Resources/Users/%s.txt", user);
    memset(save,0,10000);
    file_pointer = fopen(check,"r");
    fgets(save,9999,file_pointer);
    fclose(file_pointer);

    cJSON *user_followings = cJSON_Parse(save);
    cJSON *followings = cJSON_GetObjectItemCaseSensitive(user_followings,"followings");

    for (int i = 0; i < cJSON_GetArraySize(followings); ++i) {
        members = cJSON_GetArrayItem(followings,i);
        if (strcmp(members->valuestring,search) == 0){
            cJSON_DeleteItemFromArray(followings,i);
            break;
        }
    }

    file_pointer = fopen(check,"w");
    fprintf(file_pointer,"%s",cJSON_PrintUnformatted(user_followings));
    fclose(file_pointer);
    cJSON_Delete(user_followings);

    sprintf(response,"{\"type\":\"Successful\",\"message\":\"You unfollow this account.\"}");
    printf("{\"type\":\"Successful\",\"message\":\"You unfollow this account.\"}\n");

    return response;
}

// follow an account

char *follow(char buffer[10000]) {

    char *response = (char *)calloc(10000,1);
    char token[100];
    char search[100];
    memset(token,0,100);
    memset(search,0,100);

    sscanf(buffer,"%*s%s%s",token,search);

    token[strlen(token)-1] = '\0';

    char user[100];
    memset(user,0,100);

    cJSON *online = cJSON_GetObjectItemCaseSensitive(online_data,"online");
    cJSON *element;

    for (int i = 0; i < cJSON_GetArraySize(online); ++i) {
        element = cJSON_GetArrayItem(online,i);
        if (!strcmp(token,element->valuestring)){
            strcpy(user,element->string);
            break;
        }
    }
    if (!strlen(user)){
        sprintf(response,"{\"type\":\"Error\",\"message\":\"Token is invalid.\"}");
        printf("{\"type\":\"Error\",\"message\":\"Token is invalid.\"}\n");
        return response;
    }

    char check[110];
    memset(check, 0, 110);
    sprintf(check, "../Resources/Users/%s.txt", search);

    if (access(check,F_OK)){
        sprintf(response,"{\"type\":\"Error\",\"message\":\"This username is not valid.\"}");
        printf("{\"type\":\"Error\",\"message\":\"This username is not valid.\"}\n");
        return response;
    }

    char save[10000];
    memset(save,0,10000);

    FILE *file_pointer;
    file_pointer = fopen(check,"r");
    fgets(save,9999,file_pointer);
    fclose(file_pointer);

    cJSON *user_json = cJSON_Parse(save);
    cJSON *followers = cJSON_GetObjectItemCaseSensitive(user_json,"followers");
    cJSON *members;
    int isfollow = 0;

    for (int i = 0; i < cJSON_GetArraySize(followers); ++i) {
        members = cJSON_GetArrayItem(followers,i);
        if (strcmp(members->valuestring,user) == 0){
            isfollow = 1;
            break;
        }
    }

    if (isfollow == 1){
        sprintf(response,"{\"type\":\"Error\",\"message\":\"You are following this account.\"}");
        printf("{\"type\":\"Error\",\"message\":\"You are following this account.\"}\n");
        return response;
    }

    cJSON_AddItemToArray(followers,cJSON_CreateString(user));
    file_pointer = fopen(check,"w");
    fprintf(file_pointer,"%s",cJSON_PrintUnformatted(user_json));
    fclose(file_pointer);
    cJSON_Delete(user_json);

    memset(check, 0, 110);
    sprintf(check, "../Resources/Users/%s.txt", user);
    memset(save,0,10000);
    file_pointer = fopen(check,"r");
    fgets(save,9999,file_pointer);
    fclose(file_pointer);

    cJSON *user_followings = cJSON_Parse(save);
    cJSON *followings = cJSON_GetObjectItemCaseSensitive(user_followings,"followings");

    cJSON_AddItemToArray(followings,cJSON_CreateString(search));
    file_pointer = fopen(check,"w");
    fprintf(file_pointer,"%s",cJSON_PrintUnformatted(user_followings));
    fclose(file_pointer);
    cJSON_Delete(user_followings);

    sprintf(response,"{\"type\":\"Successful\",\"message\":\"Now you follow this account.\"}");
    printf("{\"type\":\"Successful\",\"message\":\"Now you follow this account.\"}\n");

    return response;
}

// search an account

char *search(char buffer[10000]) {

    char *response = (char *)calloc(10000,1);
    char token[100];
    char search[100];
    memset(token,0,100);
    memset(search,0,100);

    sscanf(buffer,"%*s%s%s",token,search);

    token[strlen(token)-1] = '\0';

    char user[100];
    memset(user,0,100);

    cJSON *online = cJSON_GetObjectItemCaseSensitive(online_data,"online");
    cJSON *element;

    for (int i = 0; i < cJSON_GetArraySize(online); ++i) {
        element = cJSON_GetArrayItem(online,i);
        if (!strcmp(token,element->valuestring)){
            strcpy(user,element->string);
            break;
        }
    }
    if (!strlen(user)){
        sprintf(response,"{\"type\":\"Error\",\"message\":\"Token is invalid.\"}");
        printf("{\"type\":\"Error\",\"message\":\"Token is invalid.\"}\n");
        return response;
    }

    if (!strcmp(user,search)){
        sprintf(response,"{\"type\":\"Error\",\"message\":\"This is your own profile! You can see your profile in the profile section.\"}");
        printf("{\"type\":\"Error\",\"message\":\"This is your own profile! You can see your profile in the profile section.\"}\n");
        return response;
    }

    char check[110];
    memset(check, 0, 110);
    sprintf(check, "../Resources/Users/%s.txt", search);

    if (access(check,F_OK)){
        sprintf(response,"{\"type\":\"Error\",\"message\":\"This username is not valid.\"}");
        printf("{\"type\":\"Error\",\"message\":\"This username is not valid.\"}\n");
        return response;
    }

    char save[10000];
    memset(save,0,10000);

    FILE *file_pointer;
    file_pointer = fopen(check,"r");
    fgets(save,9999,file_pointer);
    fclose(file_pointer);

    cJSON *user_json = cJSON_Parse(save);
    int number_of_followers = cJSON_GetArraySize(cJSON_GetObjectItemCaseSensitive(user_json,"followers"));
    int number_of_followings = cJSON_GetArraySize(cJSON_GetObjectItemCaseSensitive(user_json,"followings"));
    cJSON *follower = cJSON_GetObjectItemCaseSensitive(user_json,"followers");
    cJSON *follower_element;
    int isfollow = 0;

    for (int i = 0; i < cJSON_GetArraySize(follower); ++i) {
        follower_element = cJSON_GetArrayItem(follower,i);
        if (!strcmp(follower_element->valuestring,user)){
            isfollow = 1;
            break;
        }
    }

    cJSON *username = cJSON_GetObjectItemCaseSensitive(user_json,"username");
    cJSON *bio = cJSON_GetObjectItemCaseSensitive(user_json,"bio");
    cJSON *personal_tweet = cJSON_GetObjectItemCaseSensitive(user_json,"personalTweets");
    cJSON *alltweets = cJSON_CreateArray();
    cJSON *tweet;
    cJSON *parser;
    for (int i = 0; i < cJSON_GetArraySize(personal_tweet); ++i) {
        tweet = cJSON_GetArrayItem(personal_tweet,i);
        memset(check, 0, 110);
        sprintf(check, "../Resources/Tweets/%d.txt", tweet->valueint);
        memset(save,0,10000);
        file_pointer = fopen(check,"r");
        fgets(save,9999,file_pointer);
        fclose(file_pointer);
        parser = cJSON_Parse(save);
        cJSON_AddItemToArray(alltweets,parser);
    }
    cJSON *response_json = cJSON_CreateObject();
    cJSON_AddStringToObject(response_json,"type","Profile");
    cJSON *message = cJSON_CreateObject();
    cJSON_AddStringToObject(message,"username",username->valuestring);
    cJSON_AddStringToObject(message,"bio",bio->valuestring);
    cJSON_AddNumberToObject(message,"numberOfFollowers",number_of_followers);
    cJSON_AddNumberToObject(message,"numberOfFollowings",number_of_followings);
    cJSON_AddItemToObject(message,"allTweets",alltweets);
    if (isfollow == 1){
        cJSON_AddStringToObject(message,"followStatus","Followed");
    }else{
        cJSON_AddStringToObject(message,"followStatus","NotFollowed");
    }
    cJSON_AddItemToObject(response_json,"message",message);

    sprintf(response,"%s",cJSON_PrintUnformatted(response_json));
    printf("%s",cJSON_PrintUnformatted(response_json));

    return response;

}

// tweet profile

char *profile(char buffer[10000]) {
    char *response = (char *)calloc(10000,1);
    char token[100];
    memset(token,0,100);

    sscanf(buffer,"%*s%s",token);

    char user[100];
    memset(user,0,100);

    cJSON *online = cJSON_GetObjectItemCaseSensitive(online_data,"online");
    cJSON *element;

    for (int i = 0; i < cJSON_GetArraySize(online); ++i) {
        element = cJSON_GetArrayItem(online,i);
        if (!strcmp(token,element->valuestring)){
            strcpy(user,element->string);
            break;
        }
    }
    if (!strlen(user)){
        sprintf(response,"{\"type\":\"Error\",\"message\":\"Token is invalid.\"}");
        printf("{\"type\":\"Error\",\"message\":\"Token is invalid.\"}\n");
        return response;
    }

    char check[110];
    memset(check, 0, 110);
    sprintf(check, "../Resources/Users/%s.txt", user);

    char save[10000];
    memset(save,0,10000);

    FILE *file_pointer;
    file_pointer = fopen(check,"r");
    fgets(save,9999,file_pointer);
    fclose(file_pointer);

    cJSON *user_json = cJSON_Parse(save);
    int number_of_followers = cJSON_GetArraySize(cJSON_GetObjectItemCaseSensitive(user_json,"followers"));
    int number_of_followings = cJSON_GetArraySize(cJSON_GetObjectItemCaseSensitive(user_json,"followings"));
    cJSON *username = cJSON_GetObjectItemCaseSensitive(user_json,"username");
    cJSON *bio = cJSON_GetObjectItemCaseSensitive(user_json,"bio");
    cJSON *personal_tweet = cJSON_GetObjectItemCaseSensitive(user_json,"personalTweets");
    cJSON *alltweets = cJSON_CreateArray();
    cJSON *tweet;
    cJSON *parser;
    for (int i = 0; i < cJSON_GetArraySize(personal_tweet); ++i) {
        tweet = cJSON_GetArrayItem(personal_tweet,i);
        memset(check, 0, 110);
        sprintf(check, "../Resources/Tweets/%d.txt", tweet->valueint);
        memset(save,0,10000);
        file_pointer = fopen(check,"r");
        fgets(save,9999,file_pointer);
        fclose(file_pointer);
        parser = cJSON_Parse(save);
        cJSON_AddItemToArray(alltweets,parser);
    }
    cJSON *response_json = cJSON_CreateObject();
    cJSON_AddStringToObject(response_json,"type","Profile");
    cJSON *message = cJSON_CreateObject();
    cJSON_AddStringToObject(message,"username",username->valuestring);
    cJSON_AddStringToObject(message,"bio",bio->valuestring);
    cJSON_AddNumberToObject(message,"numberOfFollowers",number_of_followers);
    cJSON_AddNumberToObject(message,"numberOfFollowings",number_of_followings);
    cJSON_AddItemToObject(message,"allTweets",alltweets);
    cJSON_AddItemToObject(response_json,"message",message);

    sprintf(response,"%s",cJSON_PrintUnformatted(response_json));
    printf("%s",cJSON_PrintUnformatted(response_json));

    cJSON_Delete(response_json);
    cJSON_Delete(user_json);
    return response;
}

// set bio

char *set(char buffer[10000]) {
    char *response = (char *)calloc(10000,1);
    char token[100];
    char bio[100];
    memset(token,0,100);
    memset(bio,0,100);

    sscanf(buffer,"%*s%*s%s%*c%[^\n]s",token,bio);

    token[strlen(token)-1] = '\0';

    char user[100];
    memset(user,0,100);

    cJSON *online = cJSON_GetObjectItemCaseSensitive(online_data,"online");
    cJSON *element;

    for (int i = 0; i < cJSON_GetArraySize(online); ++i) {
        element = cJSON_GetArrayItem(online,i);
        if (!strcmp(token,element->valuestring)){
            strcpy(user,element->string);
            break;
        }
    }
    if (!strlen(user)){
        sprintf(response,"{\"type\":\"Error\",\"message\":\"Token is invalid.\"}");
        printf("{\"type\":\"Error\",\"message\":\"Token is invalid.\"}\n");
        return response;
    }

    char check[110];
    memset(check, 0, 110);
    sprintf(check, "../Resources/Users/%s.txt", user);

    char save[10000];
    memset(save,0,10000);

    FILE *file_pointer;
    file_pointer = fopen(check,"r");
    fgets(save,9999,file_pointer);
    fclose(file_pointer);

    cJSON *user_json = cJSON_Parse(save);
    cJSON *bio_json = cJSON_CreateString(bio);
    cJSON_ReplaceItemInObjectCaseSensitive(user_json,"bio",bio_json);

    file_pointer = fopen(check,"w");
    fprintf(file_pointer,"%s",cJSON_PrintUnformatted(user_json));
    fclose(file_pointer);
    cJSON_Delete(user_json);

    sprintf(response,"{\"type\":\"Successful\",\"message\":\"Bio changed.\"}");
    printf("{\"type\":\"Successful\",\"message\":\"Bio changed.\"}\n");
    return response;
}

// change password

char *change(char buffer[10000]) {
    char *response = (char *)calloc(10000,1);
    char token[100];
    char old_pass[100];
    char new_pass[100];
    memset(token,0,100);
    memset(new_pass,0,100);
    memset(old_pass,0,100);

    sscanf(buffer,"%*s%*s%s%s%s",token,old_pass,new_pass);

    token[strlen(token)-1] = '\0';
    old_pass[strlen(old_pass)-1] = '\0';

    char user[100];
    memset(user,0,100);

    cJSON *online = cJSON_GetObjectItemCaseSensitive(online_data,"online");
    cJSON *element;

    for (int i = 0; i < cJSON_GetArraySize(online); ++i) {
        element = cJSON_GetArrayItem(online,i);
        if (!strcmp(token,element->valuestring)){
            strcpy(user,element->string);
            break;
        }
    }
    if (!strlen(user)){
        sprintf(response,"{\"type\":\"Error\",\"message\":\"Token is invalid.\"}");
        printf("{\"type\":\"Error\",\"message\":\"Token is invalid.\"}\n");
        return response;
    }

    if (strlen(new_pass)>16){
        sprintf(response,"{\"type\":\"Error\",\"message\":\"Your password is too long please try again.\"}");
        printf("{\"type\":\"Error\",\"message\":\"Your password is too long please try again.\"}\n");
        return response;
    }

    char check[110];
    memset(check, 0, 110);
    sprintf(check, "../Resources/Users/%s.txt", user);

    char save[10000];
    memset(save,0,10000);

    FILE *file_pointer;
    file_pointer = fopen(check,"r");
    fgets(save,9999,file_pointer);
    fclose(file_pointer);

    cJSON *user_json = cJSON_Parse(save);
    cJSON *password = cJSON_GetObjectItemCaseSensitive(user_json,"password");

    if (strcmp(old_pass,password->valuestring) != 0){
        sprintf(response,"{\"type\":\"Error\",\"message\":\"Incorrect password.\"}");
        printf("{\"type\":\"Error\",\"message\":\"Incorrect password..\"}\n");
        return response;
    }

    cJSON *new_pass_json = cJSON_CreateString(new_pass);
    cJSON_ReplaceItemInObjectCaseSensitive(user_json,"password",new_pass_json);

    file_pointer = fopen(check,"w");
    fprintf(file_pointer,"%s",cJSON_PrintUnformatted(user_json));
    fclose(file_pointer);
    cJSON_Delete(user_json);

    sprintf(response,"{\"type\":\"Successful\",\"message\":\"Password changed.\"}");
    printf("{\"type\":\"Successful\",\"message\":\"Password changed.\"}\n");
    return response;
}

// logout

char *logout(char buffer[10000]) {

    char *response = (char *)calloc(10000,1);
    char token[100];
    memset(token,0,100);

    sscanf(buffer,"%*s%s",token);
    int count = 1;

    cJSON *online = cJSON_GetObjectItemCaseSensitive(online_data,"online");
    cJSON *element;

    for (int i = 0; i < cJSON_GetArraySize(online); ++i) {
        element = cJSON_GetArrayItem(online,i);
        if (!strcmp(token,element->valuestring)){
            cJSON_DeleteItemFromArray(online,i);
            break;
        }else{
            count += 1;
        }
    }

    if (count == cJSON_GetArraySize(online)){
        sprintf(response,"{\"type\":\"Error\",\"message\":\"Token is invalid.\"}");
        printf("{\"type\":\"Error\",\"message\":\"Token is invalid.\"}\n");
        return response;
    }

    sprintf(response,"{\"type\":\"Successful\",\"message\":\"Logged out\"}");
    printf("{\"type\":\"Successful\",\"message\":\"Logged out\"}\n");

    return response;
}

// send tweet

char *send_tweet(char buffer[10000]) {
    char *response = (char *)calloc(10000,1);

    char token[100];
    char tweet[1000];
    memset(token,0,100);
    memset(tweet,0,100);

    sscanf(buffer,"%*s%*s%s%*c%[^\n]s",token,tweet);

    token[strlen(token)-1] = '\0';

    char author[100];
    memset(author,0,100);

    cJSON *online = cJSON_GetObjectItemCaseSensitive(online_data,"online");
    cJSON *element;

    for (int i = 0; i < cJSON_GetArraySize(online); ++i) {
        element = cJSON_GetArrayItem(online,i);
        if (!strcmp(token,element->valuestring)){
            strcpy(author,element->string);
            break;
        }
    }

    char check[110];
    int i = 1;
    sprintf(check, "../Resources/Tweets/%d.txt", i);

    while (!access(check,F_OK)) {
        i += 1;
        memset(check, 0, 110);
        sprintf(check, "../Resources/Tweets/%d.txt", i);
    }

    cJSON  *tweet_json;
    tweet_json = cJSON_CreateObject();
    cJSON_AddNumberToObject(tweet_json,"id",i);
    cJSON_AddStringToObject(tweet_json,"author",author);
    cJSON_AddStringToObject(tweet_json,"content",tweet);
    cJSON_AddNumberToObject(tweet_json,"likes",0);
    cJSON *comment;
    comment = cJSON_CreateObject();
    cJSON_AddItemToObject(tweet_json,"comments",comment);

    FILE *file_pointer;
    file_pointer = fopen(check,"w");
    fprintf(file_pointer,"%s",cJSON_PrintUnformatted(tweet_json));
    fclose(file_pointer);
    cJSON_Delete(tweet_json);

    memset(check, 0, 110);
    sprintf(check, "../Resources/Users/%s.txt", author);

    char save[10000];
    memset(save,0,10000);

    file_pointer = fopen(check,"r");
    fgets(save,9999,file_pointer);
    fclose(file_pointer);

    cJSON *user = cJSON_Parse(save);
    cJSON *personal_tweets = cJSON_GetObjectItemCaseSensitive(user,"personalTweets");
    cJSON *tweet_id = cJSON_CreateNumber(i);
    cJSON_AddItemToArray(personal_tweets,tweet_id);

    file_pointer = fopen(check,"w");
    fprintf(file_pointer,"%s",cJSON_PrintUnformatted(user));
    fclose(file_pointer);
    cJSON_Delete(user);

    sprintf(response,"{\"type\":\"Successful\",\"message\":\"Tweet received\"}");
    printf("{\"type\":\"Successful\",\"message\":\"Tweet received\"}\n");

    return response;
}

// log in function and token creator

char *randstring() {

    int length = 32;

    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_-~";
    char *randomString = NULL;
    srand(time(0));

    if (length) {
        randomString = malloc(sizeof(char) * (length +1));

        if (randomString) {
            for (int n = 0;n < length;n++) {
                int key = rand() % (int)(sizeof(charset) -1);
                randomString[n] = charset[key];
            }

            randomString[length] = '\0';
        }
    }

    return randomString;
}

char *login(char buffer[10000]) {
    char *response = (char *)calloc(10000,1);

    char username[100];
    char password[100];
    memset(username,0,100);
    memset(password,0,100);

    sscanf(buffer,"%*s%s%s",username,password);

    username[strlen(username)-1] = '\0';

    char check[110];
    memset(check,0,110);

    sprintf(check,"../Resources/Users/%s.txt",username);

    if (access(check,F_OK)){
        sprintf(response,"{\"type\":\"Error\",\"message\":\"This username does not exist.\"}");
        printf("{\"type\":\"Error\",\"message\":\"This username does not exist.\"}\n");
        return response;
    }

    char data[10000];
    memset(data,0,10000);

    FILE *user_file;
    user_file = fopen(check,"r");
    fgets(data,9999,user_file);
    fclose(user_file);

    cJSON *root = cJSON_Parse(data);
    cJSON *pass = cJSON_GetObjectItemCaseSensitive(root,"password");

    if (strcmp(password,pass->valuestring) != 0){
        sprintf(response,"{\"type\":\"Error\",\"message\":\"Incorrect password.\"}");
        printf("{\"type\":\"Error\",\"message\":\"Incorrect password.\"}\n");
        return response;
    }
    cJSON_Delete(root);

    cJSON *online = cJSON_GetObjectItemCaseSensitive(online_data,"online");
    cJSON *element;

    for (int i = 0; i < cJSON_GetArraySize(online); ++i) {
       element = cJSON_GetArrayItem(online,i);
        if (!strcmp(username,element->string)){
            sprintf(response,"{\"type\":\"Error\",\"message\":\"This username has already logged in.\"}");
            printf("{\"type\":\"Error\",\"message\":\"This username has already logged in.\"}\n");
            return response;
        }
    }

    char *token;
    token = randstring();

    sprintf(response,"{\"type\":\"Token\",\"message\":\"%s\"}",token);
    printf("{\"type\":\"Token\",\"message\":\"%s\"}\n",token);

    cJSON_AddStringToObject(online,username,token);

    free(token);
    return response;
}

// signup function

char *signup(char buffer[10000]) {

    char *response = (char *)calloc(10000,1);

    char username[100];
    char password[100];
    memset(username,0,100);
    memset(password,0,100);

    sscanf(buffer,"%*s%s%s",username,password);

    username[strlen(username)-1] = '\0';

    char check[110];
    memset(check,0,110);

    sprintf(check,"../Resources/Users/%s.txt",username);

    if (!access(check,F_OK)){
        sprintf(response,"{\"type\":\"Error\",\"message\":\"This username is already taken.\"}");
        printf("{\"type\":\"Error\",\"message\":\"This username is already taken.\"}\n");
        return response;
    }

    if (strlen(password)>16){
        sprintf(response,"{\"type\":\"Error\",\"message\":\"Your password is too long please try again.\"}");
        printf("{\"type\":\"Error\",\"message\":\"Your password is too long please try again.\"}\n");
        return response;
    }


    FILE *username_file;
    username_file = fopen(check,"w");
    if (username_file != NULL) {
        fprintf(username_file,
                "{\"username\":\"%s\",\"password\":\"%s\",\"bio\":\"\",\"followers\":[],\"followings\":[],\"personalTweets\":[],\"seenTweets\":[],\"likedTweets\":[]}",
                username, password);
        fclose(username_file);
    }else{
        printf("Can not open the file\n");
        sprintf(response,"{\"type\":\"Error\",\"message\":\"An error occurred\"}");
        return response;
    }

    sprintf(response,"{\"type\":\"Successful\",\"message\":\"\"}");
    printf("{\"type\":\"Successful\",\"message\":\"\"}\n");
    return response;
}

//main Function

int main()
{
    int initial = initialize();
    if (initial == INVALID_SOCKET){
        return 0;
    }else{
        printf("Listening on port 12345 ...\n");
    }

    online_data = cJSON_CreateObject();
    cJSON_AddArrayToObject(online_data,"online");
    start(initial);

    return 0;
}
