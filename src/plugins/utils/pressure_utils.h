#ifndef __PRESSURE_DEFINES_H
#define __PRESSURE_DEFINES_H

struct pressure_values {
        float avg10;
        float avg60;
        float avg300;
};

struct pressure_values *
parse_pressure(char *what, char *pfn, struct pressure_values *pp);

#endif /* __PRESSURE_DEFINES_H */
