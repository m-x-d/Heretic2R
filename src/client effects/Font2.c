// TMix (c) Raven Software (Pagan).
// 96 textures processed.
// 67% of total area consumed
// 0 of textures were unable to be placed.

typedef struct glxy_s
{
	float	xl, yt, xr, yb;
	int		w, h, baseline;
} glxy_t;

#define glxy(label, x, y, w, h, b)	static glxy_t label = {(x / 256.000000), (y / 128.000000), ((x + w) / 256.000000), ((y + h) / 128.000000), w, h, b };

// Format: xpos,ypos,width,height,baseline

	glxy(U_Q, 0, 0, 18, 24, 20);
	glxy(U_W, 18, 0, 20, 22, 20);
	glxy(U_K, 38, 0, 17, 24, 20);
	glxy(U_X, 56, 0, 16, 25, 21);
	glxy(U_D, 72, 0, 18, 22, 20);
	glxy(U_R, 90, 0, 16, 24, 20);
	glxy(N_0, 106, 0, 18, 21, 20);
	glxy(U_A, 124, 0, 16, 23, 20);
	glxy(U_B, 140, 0, 17, 22, 20);
	glxy(U_M, 158, 0, 17, 22, 20);
	glxy(U_O, 176, 0, 17, 22, 20);
	glxy(U_P, 194, 0, 17, 22, 20);
	glxy(U_S, 212, 0, 16, 23, 20);
	glxy(N_7, 228, 0, 14, 24, 19);
	glxy(U_H, 18, 22, 16, 22, 20);
	glxy(U_N, 72, 22, 15, 23, 20);
	glxy(U_U, 106, 22, 16, 22, 20);
	glxy(N_3, 140, 22, 15, 22, 20);
	glxy(N_9, 242, 0, 13, 24, 20);
	glxy(AT, 156, 22, 18, 19, 16);
	glxy(U_E, 174, 22, 15, 22, 20);
	glxy(U_G, 190, 22, 15, 22, 20);
	glxy(U_J, 0, 24, 14, 23, 20);
	glxy(U_T, 34, 24, 14, 23, 21);
	glxy(U_V, 88, 24, 15, 22, 20);
	glxy(N_2, 122, 24, 16, 20, 19);
	glxy(U_C, 206, 24, 14, 22, 20);
	glxy(U_F, 220, 24, 14, 22, 20);
	glxy(N_5, 234, 24, 13, 23, 19);
	glxy(U_L, 48, 26, 14, 22, 20);
	glxy(AST, 156, 42, 15, 21, 20);
	glxy(N_8, 14, 44, 15, 21, 20);
	glxy(U_Y, 104, 44, 13, 23, 21);
	glxy(L_K, 118, 44, 14, 22, 20);
	glxy(AMP, 132, 44, 18, 17, 15);
	glxy(U_Z, 172, 44, 13, 22, 20);
	glxy(L_H, 186, 44, 14, 21, 19);
	glxy(N_6, 62, 46, 14, 20, 19);
	glxy(L_M, 76, 46, 18, 16, 14);
	glxy(L_Y, 200, 46, 14, 20, 14);
	glxy(N_4, 214, 46, 13, 20, 19);
	glxy(QUERY, 0, 48, 13, 20, 19);
	glxy(L_B, 30, 48, 13, 20, 19);
	glxy(L_D, 44, 48, 13, 20, 19);
	glxy(L_P, 228, 48, 14, 19, 13);
	glxy(L_Q, 242, 48, 13, 20, 13);
	glxy(L_W, 76, 62, 18, 15, 14);
	glxy(L_G, 132, 62, 13, 19, 13);
	glxy(STRING, 146, 64, 11, 21, 20);
	glxy(PERCENT, 158, 64, 13, 18, 17);
	glxy(BSLASH, 94, 46, 10, 21, 19);
	glxy(PLUS, 14, 66, 14, 17, 16);
	glxy(FSLASH, 58, 66, 10, 21, 19);
	glxy(TILDE, 118, 66, 12, 19, 18);
	glxy(QUOTE, 172, 66, 9, 21, 20);
	glxy(L_F, 62, 26, 10, 20, 19);
	glxy(OPENCB, 248, 24, 8, 22, 16);
	glxy(CLOSECB, 68, 66, 8, 22, 16);
	glxy(L_A, 182, 66, 13, 16, 13);
	glxy(U_I, 196, 66, 7, 22, 20);
	glxy(L_N, 204, 66, 14, 15, 14);
	glxy(L_O, 28, 68, 15, 14, 13);
	glxy(L_U, 94, 68, 15, 14, 13);
	glxy(L_X, 218, 68, 15, 14, 13);
	glxy(BAR, 0, 68, 3, 26, -1);
	glxy(L_C, 44, 68, 14, 14, 13);
	glxy(L_J, 4, 68, 6, 22, 16);
	glxy(L_T, 110, 68, 8, 20, 17);
	glxy(L_V, 234, 68, 13, 15, 14);
	glxy(L_E, 76, 78, 13, 14, 13);
	glxy(HASH, 28, 82, 10, 17, 16);
	glxy(L_R, 38, 82, 11, 15, 14);
	glxy(L_S, 90, 82, 10, 16, 13);
	glxy(OPENSB, 248, 68, 7, 19, -1);
	glxy(L_Z, 130, 82, 12, 14, 13);
	glxy(SQUOTE, 10, 68, 4, 21, 20);
	glxy(L_L, 50, 82, 4, 21, 20);
	glxy(CLOSEB, 150, 44, 6, 19, 16);
	glxy(STOP, 100, 82, 5, 20, 19);
	glxy(PLING, 158, 82, 5, 20, 19);
	glxy(N_1, 54, 82, 4, 21, 20);
	glxy(OPENB, 164, 82, 6, 19, 16);
	glxy(EQUAL, 182, 82, 11, 13, -1);
	glxy(CARET, 204, 82, 10, 13, -1);
	glxy(GREAT, 214, 82, 8, 15, -1);
	glxy(L_I, 222, 82, 5, 17, 16);
	glxy(LESS, 14, 84, 7, 15, 14);
	glxy(MINUS, 228, 84, 9, 10, 9);
	glxy(UNDER, 142, 86, 14, 2, 1);
	glxy(SEMICOLON, 22, 84, 5, 11, 9);
	glxy(COLON, 14, 24, 4, 9, 8);
	glxy(COMMA, 14, 34, 4, 5, 2);

glxy_t *font2[96] =
{
	0,
	&PLING,
	&QUOTE,
	&HASH,
	&STRING,
	&PERCENT,
	&AMP,
	&SQUOTE,
	&OPENB,
	&CLOSEB,
	&AST,
	&PLUS,
	&COMMA,
	&MINUS,
	&STOP,
	&FSLASH,
	&N_0,
	&N_1,
	&N_2,
	&N_3,
	&N_4,
	&N_5,
	&N_6,
	&N_7,
	&N_8,
	&N_9,
	&COLON,
	&SEMICOLON,
	&LESS,
	&EQUAL,
	&GREAT,
	&QUERY,
	&AT,
	&U_A,
	&U_B,
	&U_C,
	&U_D,
	&U_E,
	&U_F,
	&U_G,
	&U_H,
	&U_I,
	&U_J,
	&U_K,
	&U_L,
	&U_M,
	&U_N,
	&U_O,
	&U_P,
	&U_Q,
	&U_R,
	&U_S,
	&U_T,
	&U_U,
	&U_V,
	&U_W,
	&U_X,
	&U_Y,
	&U_Z,
	&OPENSB,
	&BSLASH,
	0,
	&CARET,
	&UNDER,
	0,
	&L_A,
	&L_B,
	&L_C,
	&L_D,
	&L_E,
	&L_F,
	&L_G,
	&L_H,
	&L_I,
	&L_J,
	&L_K,
	&L_L,
	&L_M,
	&L_N,
	&L_O,
	&L_P,
	&L_Q,
	&L_R,
	&L_S,
	&L_T,
	&L_U,
	&L_V,
	&L_W,
	&L_X,
	&L_Y,
	&L_Z,
	&OPENCB,
	&BAR,
	&CLOSECB,
	&TILDE,
	0,
};

// end
