/* Program to generate text based on the context provided by input prompts.

Skeleton program written by Artem Polyvyanyy, http://polyvyanyy.com/,
September 2023, with the intention that it be modified by students
to add functionality, as required by the assignment specification.
All included code is (c) Copyright University of Melbourne, 2023.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* #DEFINE'S -----------------------------------------------------------------*/
#define SDELIM "==STAGE %d============================\n"   // stage delimiter
#define MDELIM "-------------------------------------\n"    // delimiter of -'s
#define THEEND "==THE END============================\n"    // end message
#define NOSFMT "Number of statements: %d\n"                 // no. of statements
#define NOCFMT "Number of characters: %d\n"                 // no. of chars
#define NPSFMT "Number of states: %d\n"                     // no. of states
#define TFQFMT "Total frequency: %d\n"                      // total frequency

#define CRTRNC       '\r'                          // carriage return character
#define NULLCHAR     '\0'                          // null character
#define NEWLINE      '\n'                          // newline character
#define ELLIPSE_DOT  '.'                           // dot character for ellipses

#define INI_LINES   3       // initial size of lines array
#define INI_CHARS   5       // initial size of char array
#define NULLSIZE    1       // size for null char (used in malloc)
#define MAX_CHAR    37      // maximum character allowed in one line
#define MAX_ELLIPSE 3       // maximum number of "dots" in an ellipse

#define VISITED     1       // flag to check if a state has been visited or not

/* TYPE DEFINITIONS ----------------------------------------------------------*/
typedef struct state state_t;   // a state in an automaton
typedef struct node  node_t;    // a node in a linked list

struct node {                   // a node in a linked list of transitions has
    char*           str;        // ... a transition string
    state_t*        state;      // ... the state reached via the string, and
    node_t*         next;       // ... a link to the next node in the list.
};

typedef struct {                // a linked list consists of
    node_t*         head;       // ... a pointer to the first node and
    node_t*         tail;       // ... a pointer to the last node in the list.
} list_t;

struct state {                  // a state in an automaton is characterized by
    unsigned int    id;         // ... an identifier,
    unsigned int    freq;       // ... frequency of traversal,
    int             visited;    // ... visited status flag, and
    list_t*         outputs;    // ... a list of output states.
};

typedef struct {                // an automaton consists of
    state_t*        ini;        // ... the initial state, and
    unsigned int    nid;        // ... the identifier of the next new state.
} automaton_t;

/* USEFUL FUNCTIONS ----------------------------------------------------------*/
int mygetchar(void);            // getchar() that skips carriage returns
int readinputs(char ***data, size_t ini_char); // reads input from stdin
int getline(char **line, size_t *num); // gets one line of input
int max_length(int num); // check if num has reached max length
int find_greater_num(int *num1, int *num2); // check and returns higher number
int print_ellipses(int *total_char); // prints "..."
// check for any error while processing the prompt
int check_for_flags(int *RLS, int *NSC, int *HSP, int *total_char, 
                     int *counter, int *chars_read, char *one_prompt); 
void build_automaton(char **data, int lines, automaton_t *automaton,
                     int *states, int *chars); // builds the entire automaton
void process_prompts(automaton_t *automaton, size_t ini_char); // stage 1, 2
void compress_automaton(automaton_t *automaton, int *tot_state, int *tot_freq);
void free_lines(char ***data, int lines); // free malloc'ed memory of lines
void initialise_automaton(automaton_t *automaton); // initialises automaton

/* The next 3 functions are used to traverse nodes in different circumstances */
// For process_line, stage 1 and 2
void traverse_node_1(state_t **curr_state, node_t **curr_head, 
                    int *HSF, int *NSC, int *total_char, int *counter, // within
                     int *chars_read, int *RLS, char *one_prompt);     // prompt
void traverse_node_2(state_t **curr_state, node_t **curr_head,
                     int *HSF, int *total_char, char *curr_str); // after prompt
// For compress automaton, depth first node search, stage 2
void find_and_traverse(int num_of_not_visited, state_t **curr_state,
                         state_t **parent_state, node_t **curr_head,
                         automaton_t *automaton, char *curr_str, int *ini_char);

// generate output after traversing node after prompt (traverse_node_2)
void generate_output(state_t **curr_state, int *total_char, int *NSC, int *HSP);
void do_compress(state_t **head_state, automaton_t *automaton, // do compression
                 state_t **curr_state, state_t **parent_state, // between states
                 node_t **curr_head, int *total_freq, int *compression);// x & y
void free_3_memory(state_t **state, node_t **head, char *chars); // free memory
void free_state (state_t **state); // free malloc'ed memories inside a state
void free_node (node_t **node); // free malloc'ed memories inside a node
void free_str (char *string); // free malloc'ed string
void free_automaton(automaton_t *automaton, int total_state); // free automaton
node_t *make_new_node(char character); // makes empty node and initialises it
list_t *make_new_list(void); // makes empty list and initialises it

/* WHERE IT ALL HAPPENS ------------------------------------------------------*/
int main(int argc, char *argv[]) {
    char **input; // initialise arrays of "sentences"
    int ini_char = INI_CHARS;
    int total_lines;
    int total_states; 
    int total_freq;
    automaton_t automaton; // initialise automaton used for the entire program

    /* Stage 0 */
    printf(SDELIM, 0);
    input = (char**)malloc(INI_LINES*sizeof(*input));
    total_lines = readinputs(&input, ini_char);
    build_automaton(input, total_lines, &automaton, &total_states, &total_freq);

    /* Stage 1 */
    printf(SDELIM, 1);
    process_prompts(&automaton, ini_char);

    /* Stage 2 */
    printf(SDELIM, 2);
    compress_automaton(&automaton, &total_states, &total_freq);
    printf(NPSFMT, total_states);
    printf(TFQFMT, total_freq);
    printf(MDELIM);
    process_prompts(&automaton, ini_char);

    /* The End */
    free_automaton(&automaton, total_states);
    printf(THEEND);

    return EXIT_SUCCESS;        // algorithms are fun!!!
}

/* USEFUL FUNCTIONS ----------------------------------------------------------*/

/* An improved version of getchar(); skips carriage return characters.
    NB: Adapted version of the mygetchar() function by Alistair Moffat */
int mygetchar() {
    int c;
    while ((c=getchar())==CRTRNC);
    return c;
}

/* Reads all of stage 0 inputs and stores them in an array of "sentences" 
    as well as returns the number of lines successfully read for stage 0 */
int readinputs(char ***all_chars, size_t ini_char) {
    int num_line = 0;
    char *one_line;
    size_t max_lines = INI_LINES;   
    size_t *num_chars = &ini_char;
    while (getline(&one_line, num_chars)) {
        if (num_line == max_lines) {
            /* Max lines limit reached, so realloc memory to take more lines*/
            max_lines *= 2;
            *all_chars = realloc(*all_chars, max_lines*sizeof(char *));
        }
        size_t line_len = strlen(one_line);
        (*all_chars)[num_line] = malloc((line_len + NULLSIZE) * sizeof(char));
        strcpy((*all_chars)[num_line], one_line);
        num_line++;
        /* Free malloc'ed memory */
        free(one_line);
        one_line = NULL;
    }
    return num_line;

}

/* Function designed to extract one line from stdin 
    Returns 1 if a valid line is read, 0 otherwise */
int getline(char **line, size_t *max_chars) {
    int c, len = 0;
    *max_chars = INI_CHARS;
    *line = (char*)malloc((*max_chars)*sizeof(char));
    while (((c = mygetchar()) != NEWLINE) && (c != EOF)) {
        if (len == (*max_chars)) {
            /* Max chars limit reached, so realloc memory to take more chars*/
            (*max_chars) *= 2;
            *line = realloc(*line, (*max_chars)*sizeof(char));
        }
        (*line)[len] = c;
        len++;
    }
    if (len > 0) {
        /* Valid line read */
        if (len == *max_chars) {
            *line = realloc(*line, (*max_chars + NULLSIZE)*sizeof(char));
        }
        (*line)[len] = NULLCHAR;
        return 1;
    }
    else {
        /* Only a newline character is read, so line is not valid 
            However, remember to free the malloc'ed memory */
        free(*line);
        *line = NULL;
        return 0;
    }
}

/* Builds the automaton using the input training statements */
void build_automaton(char **data, int lines, automaton_t *automaton,
                     int *states, int *chars) {
    int total_char = 0;
    initialise_automaton(automaton);
    for (int i = 0; i < lines; i++) {
        automaton->ini->freq++;
        state_t *curr_state = automaton->ini;
        for (int j = 0; data[i][j] != NULLCHAR; j++) {
            if (curr_state->outputs == NULL) {
                /* Current state is not pointing at any node, so create 
                    a list of all possible "output nodes" for it */
                curr_state->outputs = make_new_list();
            }
            node_t *curr_head = curr_state->outputs->head;
            while (curr_head != NULL) {
                /* While there is still a next available node to go into*/
                if ((curr_head->str[0] == data[i][j])) {
                    if (data[i][j + 1] != NULLCHAR) {
                        /* Char is not at the end of prompt, which means that 
                            next state is not a leaf state, so can increment
                            frequency of next state */
                        curr_head->state->freq++;
                    }
                    total_char++;
                    curr_state = curr_head->state;
                    break;
                }
                /* Character not found, so go to the next available node */
                curr_head = curr_head->next;
            }
            if (curr_head == NULL) {
                /* Character not found in any of our output nodes 
                    So create new node for our current state*/
                node_t *new_node = make_new_node(data[i][j]);
                automaton->nid++;
                
                if (curr_state->outputs->head == NULL) {
                    /* Current state is not pointing to anything at all */
                    curr_state->outputs->head = new_node;
                    curr_state->outputs->tail = new_node;
                }
                else {
                    /* Current state is pointing to at least one node */
                    curr_state->outputs->tail->next = new_node;
                    curr_state->outputs->tail = new_node;
                }
                /* Traversing into the "new" state and initialising it */
                curr_state = new_node->state;
                curr_state->outputs = NULL;
                curr_state->visited = 0;
                curr_state->id = automaton->nid;
                if (data[i][j + 1] != NULLCHAR) {
                    curr_state->freq = 1;
                }
                else {
                    /* The node that points to current state is the last 
                        character in the training statement, so current 
                        state's frequency will be 0*/
                    curr_state->freq = 0;
                }
                total_char++;
            }
        }   
    }
    int total_states = automaton->nid + 1;
    printf(NOSFMT, lines);
    printf(NOCFMT, total_char);
    printf(NPSFMT, total_states);
    free_lines(&data, lines);
    /* Pass information regarding total states and chars to the main function*/
    *states = total_states;
    *chars = total_char;
}

/* Process the prompts from input line by line by replaying it on the automaton
    and generating the continuation of the prompt*/
void process_prompts(automaton_t *automaton, size_t ini_char) {
    char *one_prompt;
    size_t *max_chars = &ini_char;
    while (getline(&one_prompt, max_chars)) {
        /* Valid line read from input, so process it */
        state_t *curr_state = automaton->ini; // reset current state 
        int total_char = 0; // counter for total character so far in one line

        /* The variables below are used as flags for 
            different scenarios within processing prompts*/
        int reached_leaf_state = 0; // 1 if traversing the prompt
                                    // leads to a leaf state
        int partially_found = 0; // 1 if part of prompt has a 
                                 // matching prefix with a node's string
        int highest_same_prefix = 0; // paired with partially_found, 
                                     // tracks highest prefix count
        int non_supported_character = 0; // 1 if prompt is not able to 
                                         // travel through any available node
        int chars_read = 0; // counter for characters read so far
        int old_len = strlen(one_prompt);
        while (chars_read < old_len) {
            /* Continues looping until all characters in prompt is processed
                 or otherwise until a disruption criteria is met */
            if (max_length(total_char)) {
                break;
            }
            int found; // 1 when prompt can traverse a state (strings match)
            int counter; // counter for chars successfully processed so far
            node_t *curr_head = curr_state->outputs->head;  
            while (curr_head != NULL) {
                /* Collects "information" from all available output states */
                found = 0;
                counter = 0;
                int same_prefix = 0; // number of same first char between
                                     // prompt and string
                for (int j = 0; j < strlen(curr_head->str); j++) {
                    if (one_prompt[j] == curr_head->str[j]) {
                        /* Every character is matching so far */
                        found = 1;
                        same_prefix++;
                    }
                    else {
                        /* Found a difference */
                        found = 0;
                        if (one_prompt[j] == NULLCHAR) {
                            /* Prompt is a full prefix of node string */
                            partially_found = 1;
                        }
                        break;
                    }
                }

                if (found) {
                    traverse_node_1(&curr_state, &curr_head, 
                    &highest_same_prefix, &non_supported_character,
                     &total_char, &counter, &chars_read, 
                     &reached_leaf_state, one_prompt);
                    break;
                }
                else if (partially_found) {
                    /* Prompt is a full prefix of node string */
                    non_supported_character = 0;
                    highest_same_prefix = find_greater_num(&same_prefix,
                                             &highest_same_prefix);
                }
                else {
                    /* Prompt contains a non-supported character */
                    non_supported_character = 1;
                    highest_same_prefix = find_greater_num(&same_prefix,
                                             &highest_same_prefix);
                }
                curr_head = curr_head->next; // Check next node
            }

            for (int i = 0; i < highest_same_prefix; i++) {
                chars_read++;
            }

            if (check_for_flags(&reached_leaf_state, 
                &non_supported_character, &highest_same_prefix, 
                &total_char, &counter, &chars_read, one_prompt)) {
                break;
            }
        }

        if (highest_same_prefix) {
            /* Prompt contains a part of the node's string, so print that */
            for (int i = 0; i < highest_same_prefix; i++) {
                printf("%c", one_prompt[i]);
            }
        }
        print_ellipses(&total_char);
        generate_output(&curr_state, &total_char, 
                        &non_supported_character, &highest_same_prefix);
        /* Free malloc'ed memory */
        free(one_prompt);
        one_prompt = NULL;
        printf("\n");
    }
}

/* Use the number of compressions provided to compress the automaton. 
    Function will stop if no more compressions are available */
void compress_automaton(automaton_t *automaton, 
                        int *total_state, int *total_freq) {
    /* Find "head" state to see where it starts branching out first. */
    state_t *curr_state = automaton->ini;
    node_t *curr_head = curr_state->outputs->head;
    state_t *head_state = NULL;
    while (head_state == NULL) {
        if (curr_head->next != NULL) {
            head_state = curr_state;
            break;
        }
        if (curr_head->state->freq == 0) {
            /* Head state does not seem to exist */
            break;
        }
        curr_state = curr_head->state;
        curr_head = curr_state->outputs->head;
    }
    /* Reset current state and current head */
    curr_state = automaton->ini;
    curr_head = curr_state->outputs->head;
    state_t *parent_state = NULL;
    int total_compression, compression = 0;
    scanf("%d\n", &total_compression);
    while (compression < total_compression) {
        if ((curr_head->next == NULL) && (curr_head->state->freq != 0)) {
            /* current state has single outgoing arc, and next state has
                one or more outgoing arcs, so compress. */
            do_compress(&head_state, automaton, &curr_state,
                 &parent_state, &curr_head, total_freq, &compression);
        }
        else if (curr_head->next != NULL) {
            /* current state has multiple outgoing arcs, so choose one */
            int ini_char = INI_CHARS;
            char *curr_str = (char*)malloc(ini_char*sizeof(char)); 
            strcpy(curr_str, curr_head->str); // Initialise curr_str
            int num_of_not_visited = 0;
            while (curr_head != NULL) {
                /* Going through all possible state options */
                if ((curr_head->state->visited) != VISITED) {
                    strcpy(curr_str, curr_head->str); // Update curr_str
                    num_of_not_visited++;
                }
                curr_head = curr_head->next;
            }
            if (num_of_not_visited == 0) {
                if (curr_state == head_state) {
                    /* Reached max amount of compressions, so exit 
                        compression loop entirely */
                    free_str(curr_str);
                    break;
                }
                /* All possible states under current state have been "visited",
                    so "close" the current state's "branch" */
                parent_state->visited = VISITED; // "close" branch under parent
                curr_state = automaton->ini; // Reset current state
                curr_head = curr_state->outputs->head; // Reset current head
                free_str(curr_str);
                continue; // re-loop!
            }

            /* For num_of_not_visited >= 1, we find and traverse! */
            find_and_traverse(num_of_not_visited, &curr_state, &parent_state,
                             &curr_head, automaton, curr_str, &ini_char);
        }
        else if (curr_head->state->freq == 0) {
            /* y has no more outgoing arcs, so close the branch */
            if (head_state != NULL) {
                parent_state->visited = VISITED;
                curr_state = automaton->ini; // Reset current state
                curr_head = curr_state->outputs->head; // Reset current head
            }
            else {
                /* Another edge case, this time automaton
                 has only one branch, so do nothing*/
                break;
            }
        }
    }
    *total_state = *total_state - compression;
}

/* The first type of traversing node, done during prompt replay
    HSP = highest_same_prefix, NSC = non_supported_character, 
    RLS = reached_leaf_state*/
void traverse_node_1(state_t **curr_state, node_t **curr_head, 
                    int *HSP, int *NSC, int *total_char, int *counter, 
                    int *chars_read, int *RLS, char *one_prompt) {
    /* Found full matching strings, turn off all prefix flags as 
        they are not needed */
    *HSP = 0;
    *NSC = 0;
    for (int j = 0; j < strlen((*curr_head)->str); j++) {
        printf("%c", one_prompt[j]);
        (*total_char)++;
        (*counter)++;
        (*chars_read)++;
    }
    int old_len = strlen(one_prompt);
    /* Updates the prompt by removing processed characters from it */
    for (int j = *counter; j < old_len + 1; j++) {
        one_prompt[j - *counter] = one_prompt[j];
    }
    (*curr_state) = (*curr_head)->state;
    if (((*curr_state)->freq) == 0) {
        /* We reached a leaf state */
        *RLS = 1;
    }
}

/* The second type of traversing node, done after prompt replay
 during output generation */
void traverse_node_2(state_t **curr_state, node_t **curr_head,
                    int *highest_same_prefix, int *total_char, char *curr_str) {
    if (*highest_same_prefix) {
        /* The state we're visiting has part of its string printed out 
            already before, so let's account for that */
        for (int i = *highest_same_prefix; i < strlen((*curr_head)->str); i++) {
            printf("%c", ((*curr_head)->str)[i]);
            (*total_char)++;
            if (max_length((*total_char))) {
                break;
            }
        }
        *highest_same_prefix = 0; // Done accounted for, continue as usual
    }
    else {
        for (int i = 0; i < strlen(curr_str); i++) {
            printf("%c", curr_str[i]);
            (*total_char)++;
            if (max_length((*total_char)) >= 37) {
                break;
            }
        }
    }
    (*curr_state) = (*curr_head)->state;
}

/* Presented with more than 1 states who have not been visited yet, this
    function finds and traverses through the correct state. 
    This function is mainly used in stage 2 - compression */
void find_and_traverse(int num_of_not_visited, state_t **curr_state,
                        state_t **parent_state, node_t **curr_head, 
                        automaton_t *automaton, char *curr_str, int *ini_char) {
    if (num_of_not_visited > 1) {
        /* There are 2 or more candidates, so choose one */
        (*curr_head) = (*curr_state)->outputs->head; // Reset current head
                                                     // to point at first node
        while ((*curr_head) != NULL) {
            if ((((*curr_head)->state->visited) != VISITED) && 
                ((strcmp((*curr_head)->str, curr_str)) < 0)) {
                /* Next state not visited, its string is lower than curr_str,
                 so set that as curr_str*/
                if (strlen((*curr_head)->str) > *ini_char) {
                    /* Space in curr_str not enough, so realloc memory */
                    *ini_char = strlen((*curr_head)->str) + NULLSIZE;
                    curr_str = realloc(curr_str, (*ini_char)*sizeof(char));
                }
                strcpy(curr_str, (*curr_head)->str);
            }
            (*curr_head) = (*curr_head)->next;
        }
    }

    if (num_of_not_visited >= 1) {
        /* We now have our curr_str and we can "visit" our next state */
        (*curr_head) = (*curr_state)->outputs->head; // Reset the current head
                                                     // to point at first node
        while ((*curr_head) != NULL) {
            if (strcmp((*curr_head)->str, curr_str) == 0) {
                /* String matches, we found our next state */
                (*curr_state) = (*curr_head)->state;
                if ((*curr_state)->outputs != NULL) {
                    /* Branching out again */
                    (*curr_head) = (*curr_state)->outputs->head;
                    (*parent_state) = (*curr_state); // Define new parent state
                }
                else {
                    /* An edge case where branch directly leads to leaf state.
                        In this case, curr_state is the parent state */
                    (*curr_state)->visited = 1; // Mark "parent state" = visited
                    (*curr_state) = automaton->ini; // Reset current state
                    (*curr_head) = (*curr_state)->outputs->head; // head too
                }
                break;
            }
            (*curr_head) = (*curr_head)->next;
        }
        free_str(curr_str);
        curr_str = NULL;
    }
}

/* Check if any of the "disruptive" flag has been raised.
     If so, take appropriate action 
    RLS = reached_leaf_state, NSC = non_supported_character,
     HSP = highest_same_prefix */
int check_for_flags(int *RLS, int *NSC, int *HSP, int *total_char, 
                    int *counter, int *chars_read, char *one_prompt) {
    if (*RLS) {
        if ((one_prompt[0]) != NULLCHAR) {
            /* Check if any character is entered past leaf state 
                If so, print the first one and stop */
            printf("%c", one_prompt[0]);
            (*total_char)++;
        }
        return 1;
    }
    else if (*NSC) {
        for (int j = 0; j < *HSP + 1; j++) {
            /* Print all matching characters AND the first non-matching
                character then stop */
            printf("%c", one_prompt[j]);
            (*total_char)++;
            (*counter)++;
            (*chars_read)++;
        }
        int old_len = strlen(one_prompt);
        for (int j = *counter; j < old_len + 1; j++) {
            /* Updates the prompt by removing processed characters from it */
            one_prompt[j - *counter] = one_prompt[j];
        }
        return 1;
    }
    else {
        return 0;
    }
}

/* After prompts have been replayed successfully, let the traversal continue
    its way into the correct states to generate output until a leaf state
    is reached. NSC = non_supported_character, HSP = highest_same_prefix */
void generate_output(state_t **curr_state, int *total_char,
                     int *NSC, int *HSP) {
    while ((*curr_state)->freq > 0) {
        /* Loop keeps going until we reach a leaf state */

        if ((max_length(*total_char)) || *NSC) {
            /* We have either reached max length or our prompt already 
                contains a non-supported character, so proceed no further */
            break;
        }

        int highest_freq = 0;
        int freq_counter = 0; // How many frequencies there are for highest_freq
        node_t *curr_head = (*curr_state)->outputs->head;
        while (curr_head != NULL) {
            if ((curr_head->state->freq) > highest_freq) {
                highest_freq = curr_head->state->freq;
                freq_counter = 1;
            }
            else if ((curr_head->state->freq) == highest_freq) {
                freq_counter++;
            }
            curr_head = curr_head->next;
        }
        curr_head = (*curr_state)->outputs->head; // Reset the current head

        if (freq_counter == 1) {
            /* We found our next state as it has the highest unique frequency */
            while (curr_head != NULL) {
                if ((curr_head->state->freq) == highest_freq) {
                    traverse_node_2(curr_state, &curr_head, HSP, 
                                    total_char, curr_head->str);
                    break;
                }
                curr_head = curr_head->next;
            }
        }
        else if (freq_counter > 1) {
            /* Highest frequency held by 2 or more states, so find one that's 
                ASCIIbetically greatest */
            int ini_char = INI_CHARS;
            // Current ASCIIbetically greatest string
            char *curr_str = (char*)malloc(ini_char*sizeof(char));
            curr_str[0] = NULLCHAR; // Initialise to one with lowest ASCII
            while (curr_head != NULL) {;
                if (((curr_head->state->freq) == highest_freq) &&
                     ((strcmp(curr_head->str, curr_str)) > 0)) {
                    /* If the node has the highest frequency AND has a string
                       which is ASCIIbetically greater than curr_str, then 
                       curr_str should be replaced with the node's string */
                    if (strlen(curr_head->str) > ini_char) {
                        ini_char = strlen(curr_head->str) + NULLSIZE;
                        curr_str = realloc(curr_str, ini_char*sizeof(char));
                    }
                    strcpy(curr_str, curr_head->str);
                }
                curr_head = curr_head->next;
            }
            curr_head = (*curr_state)->outputs->head; // Reset current head

            /* Now that we've broken ties between more than one highest 
                frequencies, we find and traverse through the correct node */
            while (curr_head != NULL) {
                if (strcmp(curr_head->str, curr_str) == 0) {
                    traverse_node_2(curr_state, &curr_head, HSP,
                                     total_char, curr_str);
                    break;
                }
                curr_head = curr_head->next;
            }
            free_str(curr_str);
        }
        (*curr_state) = curr_head->state;
    }
}

/* Do a compression at the specified x and y states */
void do_compress(state_t **head_state, automaton_t *automaton, 
                state_t **curr_state, state_t **parent_state, 
                node_t **curr_head, int *total_freq, int *compression) {
    /*current state will be denoted by x while next state will be 
        denoted by y as per assignment specifications */
    state_t *next_state = (*curr_head)->state;
    node_t *next_head = next_state->outputs->head;
    size_t str_len = strlen((*curr_head)->str); // current string length
    char *temp_char = (char*)malloc((str_len + NULLSIZE)*sizeof(char));
    strcpy(temp_char, (*curr_head)->str); // temp holds node x's string
    while (next_head != NULL) {
        strcpy((*curr_head)->str, temp_char); // "reset" current string
        int x_len = strlen((*curr_head)->str);
        int y_len = strlen(next_head->str);
        next_head->str = realloc(next_head->str, 
                        (x_len + y_len + NULLSIZE)*sizeof(char));               
        strcat((*curr_head)->str, next_head->str); // Combine 2 strings
        strcpy(next_head->str, (*curr_head)->str);
        next_head = next_head->next; // Do it again to the next node
    }
    if ((*head_state) != NULL) {
        if (next_state->id == (*head_state)->id) {
            /* Need to remove head state, so "transfer ownership" */
            (*head_state) = (*curr_state);
        }
    }
    *total_freq -= next_state->freq; // Update total frequency
    // Update x state's current pointer
    (*curr_state)->outputs->head = next_state->outputs->head;
    free_3_memory(&next_state, curr_head, temp_char);
    (*compression)++;

    (*curr_state) = automaton->ini; // Reset current state 
    (*curr_head) = (*curr_state)->outputs->head; // Reset current head
    (*parent_state) = NULL;
}

/* Free all the malloc'ed lines (input data) */
void free_lines(char ***data, int lines) {
    for (int i = 0; i < lines; i++) {
        if ((*data)[i]) {
            /* free the malloc'ed characters */
            free((*data)[i]);
            (*data)[i] = NULL;
        }
    }
    free((*data)); // free the malloc'ed lines
    (*data) = NULL;
}

/* Make and initialise a new node_t and return a pointer to it*/
node_t *make_new_node(char character) {
    node_t *node = (node_t*)malloc(sizeof(node_t));
    node->str = (char*)malloc(sizeof(char) + NULLSIZE);
    node->str[0] = character;
    node->str[1] = NULLCHAR;
    node->state = (state_t*)malloc(sizeof(state_t));
    node->next = NULL;
    return node;
}

/* Make and initialise a new list_t and return a pointer to it */
list_t *make_new_list(void) {
    list_t *list = (list_t*)malloc(sizeof(list_t));
    list->head = NULL;
    list->tail = NULL;
    return list;
}

/* Make and initialise a new automaton_t and return a pointer to it */
void initialise_automaton(automaton_t *automaton) {
    automaton->nid = 0;
    automaton->ini = (state_t*)malloc(sizeof(state_t));
    automaton->ini->id = automaton->nid;
    automaton->ini->freq = 0;
    automaton->ini->outputs = NULL;
    automaton->ini->visited = 0;
}

/* Check if num has reached allowed max length. Returns 1 if it did */
int max_length(int num) {
    if (num >= MAX_CHAR) {
        return 1;
    }
    else {
        return 0;
    }
}

/* Check if same_prefix is greater than highest_same_prefix
    Returns whichever int is greater */
int find_greater_num(int *same_prefix, int *highest_same_prefix) {
    if (*same_prefix > *highest_same_prefix) {
        return *same_prefix;
    }
    else {
        return *highest_same_prefix;
    }
}

/* Print ellipses while also account for the max length allowed */
int print_ellipses(int *total_char) {
    for (int i = 0; i < MAX_ELLIPSE; i++) {
        if (max_length(*total_char)) {
            return 1;
        }
        printf("%c", ELLIPSE_DOT);
        (*total_char)++;
    }
    return 0;
}

/* Free 3 memories at once. In particular a state, node, and char 
    This function is used when compressing the automaton*/
void free_3_memory(state_t **state, node_t **head, char *chars) {
    free_state(state);
    free_node(head);
    free_str(chars);
}

/* Free a malloc'ed state */
void free_state(state_t **state) {
    /* A state consists of a pointer to a malloc'ed output */
    if ((*state)->freq > 0) {
        /* Only non-leaf states have malloc'ed output list */
        free((*state)->outputs);
        (*state)->outputs = NULL;
    }
}

/* Free a malloc'ed node */
void free_node(node_t **node) {
    /* A node consists of pointers to: */

    /* A state */
    free((*node)->state);
    (*node)->state = NULL;

    /* A string associated with it */
    free((*node)->str);
    (*node)->str = NULL;

    /* and finally, free the node itself */
    free(*node);
    *node = NULL;
}

/* Free a malloc'ed string array*/
void free_str(char *string) {
    free(string);
    string = NULL;
}

/* Free automaton and it's contents after we're done with it */
void free_automaton(automaton_t *automaton, int total_state) {
    state_t *curr_state = automaton->ini;
    node_t *curr_head = curr_state->outputs->head;
    state_t *next_state = curr_head->state;
    while (total_state > 1) {
        /* Loop until the automaton initialised state is left */
        curr_state = automaton->ini; // Reset curr_state pointer
        curr_head = curr_state->outputs->head; // Reset head pointer
        next_state = curr_head->state; // Reset next_state pointer
        while (next_state->outputs != NULL) {
            /* Loop until we find a leaf state */
            curr_state = next_state;
            curr_head = curr_state->outputs->head;
            next_state = curr_head->state;
        }
        /* Found leaf node */
        if (curr_head->next == NULL) {
            /* Nothing relies on this state, so free to proceed as usual */
            free_state(&next_state);
            free_node(&curr_head);
            curr_state->outputs = NULL; // curr_state is now the leaf state
        }
        else {
            /* State is a direct branch from parent state, so
                update the current head pointer */
            node_t *old_head = curr_head;
            curr_state->outputs->head = curr_head->next;
            free_state(&next_state);
            free_node(&old_head);
        }
        total_state--;
    }
    /* All done, the automaton state is left */
    free_state(&(automaton->ini)); // free pointer to automaton state's list
    free(automaton->ini); // free pointer to the automaton state itself
}

/* THE END -------------------------------------------------------------------*/
