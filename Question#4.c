#include <stdio.h>
#include <string.h>

struct Book {
    int id;
    int popularity;
    int lastAccess;  // smaller value = least recently accessed
};

int main() {
    int capacity, Q;
    scanf("%d %d", &capacity, &Q);

    struct Book shelf[100];  // max capacity, can be adjusted
    int count = 0;
    int time = 1;            // access timestamp

    for (int i = 0; i < Q; i++) {
        char op[10];
        scanf("%s", op);

        if (strcmp(op, "ADD") == 0) {
            int x, y;
            scanf("%d %d", &x, &y);

            // 1. Check if the book already exists ? update popularity
            int found = -1;
            for (int j = 0; j < count; j++) {
                if (shelf[j].id == x) {
                    shelf[j].popularity = y;
                    shelf[j].lastAccess = time++;
                    found = j;
                    break;
                }
            }

            // if book existed, skip further steps
            if (found != -1)
                continue;

            // 2. If shelf full ? remove least recently accessed book
            if (count == capacity) {
                int minIndex = 0;
                for (int j = 1; j < count; j++) {
                    if (shelf[j].lastAccess < shelf[minIndex].lastAccess)
                        minIndex = j;
                }

                // remove that book
                for (int j = minIndex; j < count - 1; j++) {
                    shelf[j] = shelf[j + 1];
                }
                count--;
            }

            // 3. Add new book
            shelf[count].id = x;
            shelf[count].popularity = y;
            shelf[count].lastAccess = time++;
            count++;
        }

        else if (strcmp(op, "ACCESS") == 0) {
            int x;
            scanf("%d", &x);

            int found = -1;
            for (int j = 0; j < count; j++) {
                if (shelf[j].id == x) {
                    found = j;
                    break;
                }
            }

            if (found == -1) {
                printf("-1\n");
            } else {
                printf("%d\n", shelf[found].popularity);
                shelf[found].lastAccess = time++;  // update access time
            }
        }
    }

    return 0;
}
