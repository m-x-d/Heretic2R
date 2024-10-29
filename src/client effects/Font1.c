// TMix (c) Raven Software (Pagan).
// 96 textures processed.
// 56% of total area consumed
// 0 of textures were unable to be placed.

typedef struct glxy_s
{
	float	xl, yt, xr, yb;
	int		w, h, baseline;
} glxy_t;

#define glxy(label, x, y, w, h, b)	glxy_t label = {(x / 256.000000), (y / 128.000000), ((x + w) / 256.000000), ((y + h) / 128.000000), w, h, b };

// Format: xpos,ypos,width,height,baseline

	glxy(PERCENT, 0, 0, 17, 21, 20);
	glxy(U_W, 18, 0, 24, 14, 12);
	glxy(AT, 42, 0, 19, 18, 14);
	glxy(U_A, 62, 0, 20, 17, 16);
	glxy(U_P, 82, 0, 17, 20, 14);
	glxy(POUND, 100, 0, 19, 18, 17);
	glxy(U_M, 120, 0, 19, 17, 13);
	glxy(U_N, 140, 0, 19, 17, 15);
	glxy(U_D, 160, 0, 16, 19, 16);
	glxy(U_B, 176, 0, 16, 18, 16);
	glxy(U_K, 192, 0, 19, 15, 14);
	glxy(STRING, 212, 0, 13, 21, 18);
	glxy(U_Q, 226, 0, 18, 16, 14);
	glxy(U_R, 18, 14, 14, 20, 14);
	glxy(U_E, 192, 16, 15, 18, 15);
	glxy(U_H, 226, 16, 17, 16, 14);
	glxy(L_B, 244, 0, 11, 22, 18);
	glxy(L_H, 32, 18, 14, 19, 16);
	glxy(N_2, 46, 18, 17, 15, 13);
	glxy(QUERY, 64, 18, 13, 19, 18);
	glxy(U_L, 100, 18, 16, 16, 15);
	glxy(U_T, 116, 18, 16, 16, 14);
	glxy(U_Y, 132, 18, 14, 18, 13);
	glxy(L_K, 176, 18, 15, 17, 15);
	glxy(L_M, 78, 20, 19, 13, 9);
	glxy(L_W, 146, 20, 19, 13, 11);
	glxy(U_C, 0, 22, 15, 16, 15);
	glxy(AST, 208, 22, 12, 19, 18);
	glxy(U_V, 220, 32, 15, 16, 14);
	glxy(U_X, 236, 32, 14, 17, 14);
	glxy(U_Z, 16, 34, 15, 16, 14);
	glxy(L_Y, 46, 34, 13, 18, 9);
	glxy(HASH, 78, 34, 15, 15, 13);
	glxy(U_F, 94, 34, 14, 16, 14);
	glxy(U_G, 108, 34, 11, 19, 14);
	glxy(U_S, 146, 34, 14, 16, 14);
	glxy(N_5, 160, 34, 14, 16, 13);
	glxy(CARET, 192, 34, 13, 17, 16);
	glxy(L_F, 120, 34, 11, 19, 16);
	glxy(L_J, 132, 36, 9, 21, 15);
	glxy(L_P, 174, 36, 12, 18, 8);
	glxy(PLUS, 0, 38, 14, 15, 14);
	glxy(L_D, 32, 38, 11, 18, 15);
	glxy(L_V, 60, 38, 14, 15, 12);
	glxy(N_4, 206, 42, 14, 14, 12);
	glxy(U_U, 220, 48, 14, 14, 12);
	glxy(FSLASH, 14, 50, 10, 18, 17);
	glxy(N_3, 74, 50, 12, 16, 13);
	glxy(OPENB, 86, 50, 9, 18, 17);
	glxy(LESS, 142, 50, 13, 14, 13);
	glxy(GREAT, 156, 50, 13, 14, 13);
	glxy(OPENSB, 96, 50, 9, 18, 17);
	glxy(BSLASH, 234, 50, 9, 18, 17);
	glxy(U_J, 244, 50, 10, 17, 14);
	glxy(L_G, 44, 52, 10, 17, 8);
	glxy(N_8, 186, 52, 13, 14, 12);
	glxy(L_T, 0, 54, 11, 16, 13);
	glxy(N_9, 54, 54, 11, 16, 14);
	glxy(CLOSECB, 106, 54, 9, 18, 17);
	glxy(CLOSESB, 116, 54, 9, 17, 16);
	glxy(L_A, 170, 54, 14, 12, 9);
	glxy(AMP, 24, 56, 11, 15, 13);
	glxy(N_7, 200, 56, 11, 15, 13);
	glxy(PLING, 66, 54, 8, 18, 16);
	glxy(U_I, 126, 58, 10, 15, 14);
	glxy(QUOTE, 36, 56, 8, 17, 16);
	glxy(L_I, 212, 56, 8, 17, 14);
	glxy(CLOSEB, 220, 62, 8, 17, 16);
	glxy(L_N, 136, 64, 14, 11, 9);
	glxy(L_Q, 150, 64, 11, 14, 8);
	glxy(EQUAL, 162, 66, 14, 11, 10);
	glxy(N_6, 74, 66, 10, 15, 13);
	glxy(U_O, 176, 66, 10, 15, 13);
	glxy(L_Z, 186, 66, 13, 12, 9);
	glxy(OPENCB, 12, 68, 7, 18, 13);
	glxy(L_U, 84, 68, 13, 11, 9);
	glxy(L_X, 228, 68, 12, 12, 9);
	glxy(N_0, 240, 68, 9, 15, 13);
	glxy(BAR, 250, 22, 3, 21, 17);
	glxy(L_L, 98, 68, 7, 16, 14);
	glxy(N_1, 0, 70, 8, 15, 13);
	glxy(TILDE, 44, 70, 15, 8, 7);
	glxy(APOST, 250, 68, 5, 17, 16);
	glxy(SQUOTE, 60, 70, 5, 17, 16);
	glxy(L_E, 166, 20, 10, 12, 9);
	glxy(L_C, 20, 72, 9, 12, 9);
	glxy(L_O, 106, 72, 9, 11, 9);
	glxy(SEMICOLON, 186, 36, 5, 14, 11);
	glxy(L_S, 30, 72, 6, 13, 10);
	glxy(L_R, 116, 72, 9, 10, 8);
	glxy(STOP, 66, 72, 5, 13, 12);
	glxy(UNDER, 200, 72, 12, 5, 0);
	glxy(COLON, 36, 74, 5, 12, 11);
	glxy(MINUS, 126, 74, 8, 8, 7);
	glxy(COMMA, 220, 22, 5, 7, 4);

glxy_t *font1[96] =
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
	&CLOSESB,
	&CARET,
	&UNDER,
	&APOST,
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
	&POUND,
};

// end
