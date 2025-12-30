#ifndef MCMINI_PROGRESS_H
#define MCMINI_PROGRESS_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Weak declaration.
 * If McMini is not present, this resolves to NULL.
 * If McMini is present, libmcmini.so provides the definition.
 */
__attribute__((weak))
void mc_report_progress(void);

#ifdef __cplusplus
}
#endif

void MC_PROGRESS(void)
{
    if (mc_report_progress) {
        mc_report_progress();
    }
}

#endif /* MCMINI_PROGRESS_H */

