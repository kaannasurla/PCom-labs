#define DLE (char)0
#define STX (char)2
#define ETX (char)3

/* Atributul este folosit pentru a anunță compilatorul să nu alinieze structura */
/* DELIM | SOURCE | DEST | PAYLOAD | DELIM */
struct __attribute__((packed)) Frame {
    char frame_delim_start[2]; /* DEL STX */
    int source; /* Identificator SURSĂ */
    int dest; /* Identificator DESTINAȚIE */
    char payload[1024]; /* Datele pe care vrem să le transmitem */
    char frame_delim_end[2]; /* DEL ETX */
};
