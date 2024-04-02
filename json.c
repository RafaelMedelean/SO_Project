#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

void generate_json(const char *path, const char *relative_path, json_object *jarray) {
    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror("Eroare la deschiderea directorului");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            // Construiește calea completă și calea relativă pentru elementul curent
            char full_path[1024];
            char new_relative_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
            snprintf(new_relative_path, sizeof(new_relative_path), "%s/%s", relative_path, entry->d_name);

            // Creează un obiect JSON pentru elementul curent
            json_object *jobj = json_object_new_object();
            json_object *jname = json_object_new_string(entry->d_name);
            json_object *jtype = json_object_new_string(entry->d_type == DT_DIR ? "director" : "fisier");
            json_object *jrelative_path = json_object_new_string(new_relative_path);

            json_object_object_add(jobj, "nume", jname);
            json_object_object_add(jobj, "tip", jtype);
            json_object_object_add(jobj, "cale", jrelative_path);

            json_object_array_add(jarray, jobj);

            if (entry->d_type == DT_DIR) {
                // Apel recursiv cu noua cale relativă
                generate_json(full_path, new_relative_path, jarray);
            }
        }
    }
    closedir(dir);
}

void save_json_to_file(json_object *jobj, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Eroare la deschiderea fișierului pentru scriere");
        return;
    }
    const char *json_string = json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PRETTY);
    fprintf(file, "%s", json_string);
    fclose(file);
}

char *find_last_json_file(const char *directory, const char *exclude_file_name) {
    DIR *dir;
    struct dirent *entry;
    char *last_file_name = NULL;
    dir = opendir(directory);
    if (!dir) {
        perror("Eroare la deschiderea directorului");
        return NULL;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && strcmp(entry->d_name, exclude_file_name) != 0) {
            const char *ext = strrchr(entry->d_name, '.');
            if (!ext || strcmp(ext, ".json") != 0) continue;
            
            if (!last_file_name || strcmp(entry->d_name, last_file_name) > 0) {
                if (last_file_name) free(last_file_name);
                last_file_name = strdup(entry->d_name);
            }
        }
    }
    closedir(dir);

    if (last_file_name) {
        char *full_path = malloc(strlen(directory) + strlen(last_file_name) + 2);
        sprintf(full_path, "%s/%s", directory, last_file_name);
        free(last_file_name);
        return full_path;
    } else {
        return NULL;
    }
}

void compare_json_structures(json_object *current, json_object *last_saved) {

    size_t i, j;
    int found;
    size_t current_count = json_object_array_length(current);
    size_t last_count = json_object_array_length(last_saved);
    
    printf("Modificari detectate:\n");
    
    // Verifică elementele noi sau mutate
    for (i = 0; i < current_count; i++) {
        json_object *cur_item = json_object_array_get_idx(current, i);
        const char *cur_name = json_object_get_string(json_object_object_get(cur_item, "nume"));
        const char *cur_path = json_object_get_string(json_object_object_get(cur_item, "cale"));
        found = 0;
        
        for (j = 0; j < last_count; j++) {
            json_object *last_item = json_object_array_get_idx(last_saved, j);
            const char *last_name = json_object_get_string(json_object_object_get(last_item, "nume"));
            const char *last_path = json_object_get_string(json_object_object_get(last_item, "cale"));
            
            if (strcmp(cur_name, last_name) == 0 && strcmp(cur_path, last_path) == 0) {
                found = 1;
                break;
            }
        }
        
        if (!found) {
            printf("- Noul sau mutat: %s la %s\n", cur_name, cur_path);
        }
    }
    
    // Verifică elementele șterse
    for (j = 0; j < last_count; j++) {
        json_object *last_item = json_object_array_get_idx(last_saved, j);
        const char *last_name = json_object_get_string(json_object_object_get(last_item, "nume"));
        found = 0;
        
        for (i = 0; i < current_count; i++) {
            json_object *cur_item = json_object_array_get_idx(current, i);
            const char *cur_name = json_object_get_string(json_object_object_get(cur_item, "nume"));
            
            if (strcmp(cur_name, last_name) == 0) {
                found = 1;
                break;
            }
        }
        
        if (!found) {
            printf("- Sters: %s\n", last_name);
        }
    }
}

void read_and_print_json(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Eroare la deschiderea fișierului pentru citire");
        return;
    }
    
    // Mărește dimensiunea dacă este necesar
    char buffer[4096];
    fread(buffer, 1, sizeof(buffer), file);
    fclose(file);

    json_object *jobj = json_tokener_parse(buffer);
    if (jobj == NULL) {
        printf("Eroare la parsarea JSON\n");
        return;
    }

    printf("Structura JSON citită este:\n%s\n", json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jobj);
}


int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Utilizare: %s <nume_director>\n", argv[0]);
        return 1;
    }

    const char *directory_name = argv[1];
    // Directorul țintă hard-codat pentru salvarea fișierelor JSON
    const char *target_directory = "/mnt/c/Users/medel/OneDrive/Desktop/proiect so/saved_json_file/";

    struct stat st = {0};
    if (stat(target_directory, &st) == -1) {
        if (mkdir(target_directory, 0700) == -1) {
            perror("Nu s-a putut crea directorul țintă");
            return 1; // Sau gestionează eroarea cum consideri adecvat
        }
    }

    // Generarea numelui fișierului bazat pe data și ora curente
    char json_file_name[256];
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    snprintf(json_file_name, sizeof(json_file_name), "%d_%02d_%02d_%02d_%02d_%02d.json",
            tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday,
            tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);

    // Inițializează un array JSON pentru a stoca structura directorului
    json_object *jarray = json_object_new_array();

    // Generează structura JSON
    generate_json(directory_name, "", jarray); // sau "./" dacă preferi

    char full_path[512]; // Asigură-te că este suficient de mare
    snprintf(full_path, sizeof(full_path), "%s%s", target_directory, json_file_name);

    printf("Calea fișierului JSON este: %s\n", full_path);

    save_json_to_file(jarray, full_path);

    read_and_print_json(full_path);

    char *exclude_file_name = strrchr(json_file_name, '/') ? strrchr(json_file_name, '/') + 1 : json_file_name;
    char *last_json_path = find_last_json_file(target_directory, exclude_file_name);

    if (last_json_path && strcmp(last_json_path, full_path) != 0) {
        // Citim structura din ultimul fișier JSON salvat
        json_object *last_saved_json = NULL;
        FILE *file = fopen(last_json_path, "r");
        if (file) {
            char buffer[4096];
            fread(buffer, 1, sizeof(buffer), file);
            fclose(file);
            last_saved_json = json_tokener_parse(buffer);
        }

        if (last_saved_json) {
            printf("Comparând cu: %s\n", last_json_path);
            compare_json_structures(jarray, last_saved_json);
            json_object_put(last_saved_json);
        } else {
            printf("Nu s-a putut citi ultima structură salvată.\n");
        }
        free(last_json_path);
    } else {
        printf("Nu există structuri anterioare salvate pentru comparație sau structura curentă este singura existentă.\n");
    }

    json_object_put(jarray);

    return 0;
}




