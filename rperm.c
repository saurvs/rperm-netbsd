#include <sys/param.h>
#include <sys/conf.h>
#include <sys/cprng.h>
#include <sys/kmem.h>
#include <sys/module.h>
#include <sys/systm.h>
#include <sys/types.h>

#define R32MAX 4294967295
#define CMAJOR 420

/*
 * To use this device driver, run
 * 	mknod /dev/rperm c 420 0
 */

dev_type_open(rperm_open);
dev_type_close(rperm_close);
dev_type_write(rperm_write);
dev_type_read(rperm_read);

static struct cdevsw rperm_cdevsw = {
    .d_open = rperm_open,
    .d_close = rperm_close,
    .d_read = rperm_read,
    .d_write = rperm_write,
    .d_ioctl = noioctl,
    .d_stop = nostop,
    .d_tty = notty,
    .d_poll = nopoll,
    .d_mmap = nommap,
    .d_kqfilter = nokqfilter,
    .d_discard = nodiscard,
    .d_flag = D_OTHER
};

static struct rperm_softc {
    char *buf;
    size_t buf_len;
} sc;

int
rperm_open(dev_t self, int flag, int mod, struct lwp *l)
{
    return 0;
}

int
rperm_close(dev_t self, int flag, int mod, struct lwp *l)
{
    if (sc.buf != NULL) {
	kmem_free(sc.buf, sc.buf_len);
	sc.buf = NULL;
    }
    return 0;
}

int
rperm_write(dev_t self, struct uio *uio, int flags)
{
    if (sc.buf != NULL)
	kmem_free(sc.buf, sc.buf_len);
    sc.buf_len = uio->uio_iov->iov_len;
    sc.buf = (char *)kmem_alloc(sc.buf_len, KM_SLEEP);
    uiomove(sc.buf, sc.buf_len, uio);
    return 0;
}

/*
 * returns a random integer n uniformly distributed over [low, high)
 */
uint32_t rand_n(uint32_t low, uint32_t high) {
    uint32_t limit, diff, r;
    
    diff = high - low;
    limit = diff * (R32MAX/diff);
    do {
	r = cprng_strong32();
    } while (r > limit);
    return (r % diff) + low;
}

int
rperm_read(dev_t self, struct uio *uio, int flags)
{
    if (sc.buf == NULL || uio->uio_resid < sc.buf_len)
	return EINVAL;
    
    char c;
    uint32_t i, n, r;
    
    for (i = 0; i < sc.buf_len-1; i++) {
	r = rand_n(i, sc.buf_len);
	c = sc.buf[r];
	sc.buf[r] = sc.buf[i];
	sc.buf[i] = c;
    }
    uiomove(sc.buf, sc.buf_len, uio);
    return 0;
}

MODULE(MODULE_CLASS_DRIVER, rperm, NULL);

static int
rperm_modcmd(modcmd_t cmd, void *args)
{
    devmajor_t bmajor, cmajor;
    
    bmajor = -1;
    cmajor = CMAJOR;
    switch(cmd) {
        case MODULE_CMD_INIT:
            devsw_attach("rperm", NULL, &bmajor, &rperm_cdevsw, &cmajor);
	    break;
        case MODULE_CMD_FINI:
            devsw_detach(NULL, &rperm_cdevsw);
            break;
        default:
            break;
    }
    return 0;
}
