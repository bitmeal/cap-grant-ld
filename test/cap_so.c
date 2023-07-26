#include <stdio.h>
#include <cap-ng.h>

#include "cap_so.h"

int test_caps(int print_caps)
{
    int cap_counter = 0;
    for (int cap = 0; cap <= CAP_LAST_CAP; cap++)
    {
        if (capng_have_capability(CAPNG_PERMITTED, cap))
        {
            if (print_caps)
            {
                const char *cap_name = capng_capability_to_name(cap);
                if (cap_name == NULL)
                    cap_name = "unknown";
                printf("%s[%02d]%s", (cap_counter ? "," : ""), cap, cap_name);
            }

            cap_counter++;
        }
    }
    
    if (print_caps && cap_counter)
    {
        printf("\n");
    }

    return cap_counter;
}
