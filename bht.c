// Balanced search tree

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define die(...) fprintf(stderr, __VA_ARGS__), exit(1)

struct node
{
    long long key;
    void * value;
    int height;
    struct node *left, *right;
};

// Return height of node from heights of subnodes
static int height (struct node *n)
{
    int l, r;
    if (!n) return 0;
    l = n->left ? n->left->height : 0;
    r = n->right ? n->right->height : 0;
    return ((l > r) ? l : r)+1;
}

// Rotate node right
//       A                  B
//      / \               /   \
//     B   d             C     A
//    / \     - - - >   / \   / \
//   C   c             a   b c   d
//  / \
// a   b
static struct node *ror(struct node *A)
{
    struct node *B = A->left;
    A->left = B->right;
    B->right = A;
    A->height = height(A);
    B->height = height(B);
    return B;
}

// Rotate node leftt
//   A                      B
//  / \                   /   \
// a   B                 A     C
//    / \     - - ->    / \   / \
//   b   C             a   b c   d
//      / \
//     c   d
static struct node *rol(struct node *A)
{
    struct node *B = A->right;
    A->right = B->left;
    B->left = A;
    A->height = height(A);
    B->height = height(B);
    return B;
}

// Return node's balance, negative if leaning left, positive if leaning
// right, 0 if balanced.
static long long balance(struct node *n)
{
    return n ? height(n->right) - height(n->left) : 0;
}

// Insert key and value into balanced tree.
// If key already exists, just replace the value.
// *value must either be NULL or malloced.
static struct node *inskey(struct node *n, long long key, void *value)
{
    int b;

    if (!n)
    {
        n = calloc(1,sizeof(struct node));
        if (!n) die("OOM!\n");
        n->key = key;
        n->value = value;
        n->height = 1;
        return n;
    }

    if (key == n->key)
    {
        if (n->value) free(n->value);
        n->value = value;
        return n;
    }

    if (key <= n->key)
        n->left=inskey(n->left, key, value);
    else
        n->right=inskey(n->right, key, value);

    // rebalance
    n->height=height(n);
    b=balance(n);
    if (b < -1)
    {
        if (key < n->left->key) return ror(n);
        if (key >= n->left->key)
        {
            n->left=rol(n->left);
            return ror(n);
        }
    }
    else if (b > 1)
    {
        if (key > n->right->key) return rol(n);
        if (key <= n->right->key)
        {
            n->right=ror(n->right);
            return rol(n);
        }
    }
    return n;
}

// Delete key from balanced tree if it exists
struct node *delkey(struct node *n, long long key)
{
    int b;

    if (!n) return NULL;

    if (key < n->key) n->left=delkey(n->left, key);
    else if (key > n->key) n->right=delkey(n->right, key);
    else
    {
        // delete this node
        if (n->value) free(n->value);

        if (!n->left && !n->right)
        {
            // no children
            free(n);
            return NULL;
        }
        if (!n->left || !n->right)
        {
            // one child
            struct node *t = n->left?:n->right;
            *n = *t; // memcpy
            free(t);
        }
        else
        {
            // two children
            struct node *successor = n->right;
            while (successor->left) successor = successor->left;
            // import the successor key and value
            n->key = successor->key;
            n->value = successor->value;
            successor->value=NULL;
            // delete the succcessor
            n->right=delkey(n->right, n->key);
        }
    }

    // rebalance
    n->height=height(n);
    b=balance(n);
    if (b < -1)
    {
        if (balance(n->left) <= 0) return ror(n);
        n->left = rol(n->left);
        return ror(n);
    }

    if (b > 1)
    {
        if (balance(n->right) >= 0) return rol(n);
        n->right=ror(n->right);
        return rol(n);
    }

    return n;
}

// Return value for specified key.
// NULL is a legal value, so if not found return NOTFOUND.
#define NOTFOUND (void *)(-1)
void *findkey(struct node *n, long long key)
{
    while (n)
    {
        if (key < n->key) n = n->left;
        else if (key > n->key) n = n->right;
        else return n->value;
    }
    return NOTFOUND;
}

#if 1
// For this example, the key is a hashed string and the value is another string
struct node *head=NULL;

// cheap and dirty string hash
long long hash(char *key)
{
    long long h=0;
    while (*key) h=(h*33)+*key++;
    return h;
}

void insert(char *key, char *value)
{
    head=inskey(head, hash(key), value ? strdup(value) : NULL);
}

int delete(char *key)
{
    head=delkey(head, hash(key));
}

void speak(char *animal)
{
    void *v = findkey(head, hash(animal));
    if (!v) printf("The %s is quiet.\n", animal);
    else if (v == NOTFOUND) printf("The %s is a myth.\n", animal);
    else printf("The %s says %s.\n", animal, v);
}

void dump(struct node *n)
{
    if (!n) return;
    printf("%p: %lld '%s' <%p >%p\n", n, n->key, n->value?:"NULL", n->left, n->right);
    dump(n->left);
    dump(n->right);
}

int main(int argc , char *argv[])
{
    int x;
    insert("horse","neigh");
    insert("dog","bark");
    insert("cat","meow");
    insert("cow","moo");
    insert("bird","chirp");
    insert("lion","mew");
    insert("pig","oink");
    insert("rabbit",NULL);
    dump(head);
    speak("cow"); delete("cow"); speak("cow"); dump(head);
    speak("cat"); delete("cat"); speak("cat"); dump(head);
    speak("lion"); insert("lion", "roar"); speak("lion");
    speak("unicorn"); insert("unicorn","tra-la tra-la"); speak("unicorn"); dump(head);
    speak("rabbit"); insert("rabbit","I'M PETER DAMMIT"); speak("rabbit"); dump(head);
}
#endif
