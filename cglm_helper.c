#include <cglm/cglm.h>

#include "cglm_helper.h"

void glmc_mat4_print(mat4 m)
{
        for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 4; ++j) {
                        fprintf_s(stdout, "%0.2f ", m[i][j]);
                }

                fprintf_s(stdout, "\n");
                fflush(stdout);
        }
}