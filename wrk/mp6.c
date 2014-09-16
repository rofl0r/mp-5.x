#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>

#define MP6_BLK_GRAN 8

struct mp6_blk {
    size_t z;
    off_t l;
    off_t c;
    struct mp6_blk *p;
    struct mp6_blk *n;
};


wchar_t *mp6_blk_p(struct mp6_blk *b)
{
    return (wchar_t *)(b + 1);
}


struct mp6_blk *mp6_blk_new(struct mp6_blk *b, size_t z, off_t l, off_t c,
                            struct mp6_blk *p, struct mp6_blk *n)
{
    z = (z / MP6_BLK_GRAN + 1) * MP6_BLK_GRAN;

    b = realloc(b, sizeof(struct mp6_blk) + z * sizeof(wchar_t));

    b->z = z;
    b->l = l;
    b->c = c;
    b->p = p;
    b->n = n;

    if (p)
        p->n = b;
    if (n)
        n->p = b;

    return b;
}


struct mp6_blk *mp6_insert(struct mp6_blk *b, const wchar_t *d, size_t z)
{
    if (b->c == b->l) {
        if (b->z - b->l < z)
            b = mp6_blk_new(b, b->z + z, b->l, b->c, b->p, b->n);

        memcpy(mp6_blk_p(b) + b->c, d, z * sizeof(wchar_t));

        b->c += z;
        b->l += z;
    }
    else {
        mp6_insert(mp6_blk_new(NULL, 0, 0, 0, b, b->n),
                        mp6_blk_p(b) + b->c, b->l - b->c);

        b->l = b->c;
        b = mp6_insert(b, d, z);
    }

    return b;
}


struct mp6_blk *mp6_set_c(struct mp6_blk *b, int d)
{
    if (b)
        b->c = d == -1 ? b->l : 0;

    return b;
}


struct mp6_blk *mp6_fwd(struct mp6_blk *b, off_t c)
{
    off_t d = b->l - b->c;

    if (c > d) {
        if (b->n)
            b = mp6_fwd(mp6_set_c(b->n, 0), c - d);
        else
            mp6_set_c(b, -1);
    }
    else
        b->c += c;

    return b;
}


size_t mp6_get(struct mp6_blk *b, wchar_t *d, size_t z)
{
    size_t c = 0;

    if (b && z) {
        c = b->l - b->c;

        if (c > z)
            c = z;

        memcpy(d, mp6_blk_p(b) + b->c, c * sizeof(wchar_t));

        c += mp6_get(mp6_set_c(b->n, 0), d + c, z - c);
    }

    return c;
}


int main(int argc, char *argv[])
{
    struct mp6_blk *b;
    wchar_t *p;
    wchar_t tmp[2048];
    size_t z;

    b = mp6_blk_new(NULL, 0, 0, 0, NULL, NULL);

    p = L"this is an incredible test";

    b = mp6_insert(b, p, wcslen(p));
    b = mp6_insert(b, p, wcslen(p));
    b->c = 0;
    b = mp6_insert(b, p, wcslen(p));

    b->c = 0;
    z = mp6_get(b, tmp, 5);
    z = mp6_get(b, tmp, 2048);

    return 0;
}
