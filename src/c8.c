#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "c8.h"

/* defines for execinst. these could go in the header but i feel like putting
 * them here is easier */
#define NNN	(op & 0x0FFF)
#define N	(op & 0x000F)
#define X	(op & 0x0F00) >> 8
#define Y	(op & 0x00F0) >> 4
#define KK	(op & 0x00FF)

void c8_execinst(struct chip8_instance *c8)
{
	uint16_t op;

	op = c8->mem[c8->pc] << 8 | c8->mem[c8->pc + 1];
	c8->pc += 2;

	switch (op & 0xF000) {
		case 0x0000:
			switch (op & 0x000F) {
				case 0x0000: /* 00E0 */
					memset(c8->disp, 0, sizeof(int) * DISPSIZE);
					break;
				case 0x000E: /* 00EE */
					c8->pc = c8->stack[--c8->sp];
					break;
				default:
					fprintf(stderr, "Unknown opcode %.4X\n", op);
					break;
			}
			break;
		case 0x1000: /* 1NNN */
			c8->pc = NNN;
			break;
		case 0x2000: /* 2NNN */
			c8->stack[c8->sp++] = c8->pc;
			c8->pc = NNN;
			break;
		case 0x3000: /* 3XKK */
			if (c8->V[X] == KK)
				c8->pc += 2;
			break;
		case 0x4000: /* 4XKK */
			if (c8->V[X] != KK)
				c8->pc += 2;
			break;
		case 0x5000:
			switch (op & 0x000F) {
				case 0x0000: /* 5XY0 */
					if (c8->V[X] == c8->V[Y])
						c8->pc += 2;
					break;
				default:
					fprintf(stderr, "Unknown opcode %.4X\n", op);
					break;
			}
			break;
		case 0x6000: /* 6XKK */
			c8->V[X] = KK;
			break;
		case 0x7000: /* 7XKK */
			c8->V[X] += KK;
			break;
		case 0x8000:
			switch (op & 0x000F) {
				case 0x0000: /* 8XY0 */
					c8->V[X] = c8->V[Y];
					break;
				case 0x0001: /* 8XY1 */
					c8->V[X] |= c8->V[Y];
					break;
				case 0x0002: /* 8XY2 */
					c8->V[X] &= c8->V[Y];
					break;
				case 0x0003: /* 8XY3 */
					c8->V[X] ^= c8->V[Y];
					break;
				case 0x0004: /* 8XY4 */
					c8->V[0xF] = (c8->V[X] + c8->V[Y]) > 255 ? 1 : 0;
					c8->V[X] += c8->V[Y];
					break;
				case 0x0005: /* 8XY5 */
					c8->V[0xF] = (c8->V[X] > c8->V[Y]) ? 1 : 0;
					c8->V[X] -= c8->V[Y];
					break;
				case 0x0006: /* 8XY6 */
					c8->V[0xF] = ((c8->V[X] & 1) == 0) ? 0 : 1;
					c8->V[X] >>= 1;
					break; 
				case 0x0007: /* 8XY7 */
					c8->V[0xF] = (c8->V[X] > c8->V[Y]) ? 1 : 0;
					c8->V[X] = c8->V[Y] - c8->V[X];
					break;
				case 0x000E: /* 8XYE */
					c8->V[0xF] = ((c8->V[X] & 1) == 0) ? 0 : 1;
					c8->V[X] <<= 1;
					break; 
				default:
					fprintf(stderr, "Unknown opcode %.4X\n", op);
					break;
			}
			break;
		case 0x9000:
			switch (op & 0x000F) {
				case 0x0000: /* 9XY0 */
					if(c8->V[X] != c8->V[Y])
						c8->pc += 2;
					break;
				default:
					fprintf(stderr, "Unknown opcode %.4X\n", op);
					break;
			}
			break;
		case 0xA000: /* ANNN */
			c8->I = NNN;
			break;
		case 0xB000: /* BNNN */
			c8->pc = NNN + c8->V[0];
			break;
		case 0xC000: /* CXKK */
			c8->V[X] = (rand() % 255) & KK;
			break;
		/* hell */
		case 0xD000: /* DXYN */
			uint8_t px;
			c8->V[0xF] = 0;

			for (int y = 0; y < N; y++) {
				px = c8->mem[c8->I + y];
				for (int x = 0; x < 8; x++) {
					if (px & (0x80 >> x)) {
						if (c8->disp[c8->V[Y] + y][c8->V[X] + x])
							c8->V[0xF] = 1;
						c8->disp[c8->V[Y] + y][c8->V[X] + x] ^= 1;
					}
				}
			}
			c8->drawflag = 1;
			break;
		case 0xE000:
			switch (op & 0x00F0) {
				case 0x0090: /* EX9E */
					if (c8->key[c8->V[X]])
						c8->pc += 2;
					break;
				case 0x00A0: /* EXA1 */
					if (!c8->key[c8->V[X]])
						c8->pc += 2;
					break;
				default:
					fprintf(stderr, "Unknown opcode %.4X\n", op);
					break;
			}
		case 0xF000:
			switch (op & 0x00FF) {
				case 0x0007: /* FX07 */
					c8->V[X] = c8->dt;
					break;
				case 0x000A: /* FX0A */
					int done = 0;
					for (int i = 0; i < 16; i++) {
						if (c8->key[i]) {
							done = 1;
							c8->V[X] = i;
							break;
						}
					}
					if (!done)
						c8->pc -= 2; /* go back here */
					break;
				case 0x0015: /* FX15 */
					c8->dt = c8->V[X];
					break;
				case 0x0018: /* FX18 */
					c8->st = c8->V[X];
					break;
				case 0x001E: /* FX1E */
					c8->I += c8->V[X];
					break;
				case 0x0029: /* FX29 */
					/* 5 is the amount of bytes per char sprite */
					c8->I = c8->V[X] * 5 + FONTSETLOC;
					break;
				case 0x0033: /* FX33 */
					int n = c8->V[X];
					c8->mem[c8->I] = n / 100;
					c8->mem[c8->I + 1] = (n / 10) % 10;
					c8->mem[c8->I + 2] = n % 10;
					break;
				case 0x0055: /* FX55 */
					for (int i = 0; i <= X; i++)
						c8->mem[c8->I + i] = c8->V[i];
					break;
				case 0x0065: /* FX65 */
					for (int i = 0; i <= X; i++)
						c8->V[i] = c8->mem[c8->I + i];
					break;
				default:
					fprintf(stderr, "Unknown opcode %.4X\n", op);
					break;
			}
	}
}

/* there is probably a better way of doing this */
void c8_key_toggle(struct chip8_instance *c8, int keycode)
{
	switch (keycode) {
		case ALLEGRO_KEY_X:
			c8->key[0x0] ^= 1;
			break;
		case ALLEGRO_KEY_1:
			c8->key[0x1] ^= 1;
			break;
		case ALLEGRO_KEY_2:
			c8->key[0x2] ^= 1;
			break;
		case ALLEGRO_KEY_3:
			c8->key[0x3] ^= 1;
			break;
		case ALLEGRO_KEY_Q:
			c8->key[0x4] ^= 1;
			break;
		case ALLEGRO_KEY_W:
			c8->key[0x5] ^= 1;
			break;
		case ALLEGRO_KEY_E:
			c8->key[0x6] ^= 1;
			break;
		case ALLEGRO_KEY_A:
			c8->key[0x7] ^= 1;
			break;
		case ALLEGRO_KEY_S:
			c8->key[0x8] ^= 1;
			break;
		case ALLEGRO_KEY_D:
			c8->key[0x9] ^= 1;
			break;
		case ALLEGRO_KEY_Z:
			c8->key[0xA] ^= 1;
			break;
		case ALLEGRO_KEY_C:
			c8->key[0xB] ^= 1;
			break;
		case ALLEGRO_KEY_4:
			c8->key[0xC] ^= 1;
			break;
		case ALLEGRO_KEY_R:
			c8->key[0xD] ^= 1;
			break;
		case ALLEGRO_KEY_F:
			c8->key[0xE] ^= 1;
			break;
		case ALLEGRO_KEY_V:
			c8->key[0xF] ^= 1;
			break;
		case ALLEGRO_KEY_ESCAPE:
			c8->quitflag = 1;
			break;
	}
}

void c8_tick(struct chip8_instance *c8)
{
	if (c8->dt > 0)
		c8->dt--;
	if (c8->st > 0)
		c8->st--;
}

void draw_screen(struct chip8_instance *c8)
{
	al_clear_to_color(al_map_rgb(0, 0, 0));

	for (int y = 0; y < DISPHEIGHT; y++) {
		for (int x = 0; x < DISPWIDTH; x++) {
			if (c8->disp[y][x] != 0) {
				al_draw_filled_rectangle(x * PIXELSCALE,
						y * PIXELSCALE, 
						x * PIXELSCALE + PIXELSCALE,
						y * PIXELSCALE + PIXELSCALE,
						al_map_rgb(255, 255, 255));
			}
		}
	}

	al_flip_display();
}

void die(int code, const char *msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(code);
}

void read_rom(struct chip8_instance *c8, const char *path)
{
	FILE *fp;
	int i, c;

	fp = fopen(path, "r");
	if (!fp)
		die(1, "nonexistent rom");
	/* check if rom is bigger than allocated space */
	fseek(fp, 0L, SEEK_END);
	if (ftell(fp) > MEMORYSIZE - GAMERAMLOC)
		die(1, "rom too large");
	rewind(fp);

	/* read the data into ram */
	for (i = 0; (c = fgetc(fp)) != EOF; i++)
		c8->mem[GAMERAMLOC + i] = c;
}

void setup_machine(struct chip8_instance *c8)
{
	memset(c8->mem, 0, sizeof(uint8_t) * MEMORYSIZE);
	memset(c8->V, 0, sizeof(uint8_t) * 16);
	memset(c8->stack, 0, sizeof(uint16_t) * STACKSIZE);
	memset(c8->key, 0, sizeof(int) * 16);
	memset(c8->disp, 0, sizeof(int) * DISPSIZE);

	c8->I = 0;
	c8->dt = 0;
	c8->st = 0;
	c8->pc = GAMERAMLOC;
	c8->sp = 0;
	c8->drawflag = 1;
	c8->quitflag = 0;

	for (int i = 0; i < FONTSETSIZE; i++)
		c8->mem[FONTSETLOC + i] = fontset[i];

	srand(time(NULL));
}

int main(int argc, char **argv)
{
	struct chip8_instance c8;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s [rom path]\n", argv[0]);
		die(1, "no rom");
	}

	setup_machine(&c8);
	read_rom(&c8, argv[1]);
	if (!al_init())
		die(1, "al_init failure");
	if (!al_install_keyboard())
		die(1, "al_install_keyboard failure");
	if (!al_init_primitives_addon())
		die(1, "al_init_primitives_addon failure");
	ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60.0);
	ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
	ALLEGRO_DISPLAY *disp = al_create_display(DISPWIDTH * PIXELSCALE,
			DISPHEIGHT * PIXELSCALE);

	al_register_event_source(queue, al_get_keyboard_event_source());
	al_register_event_source(queue, al_get_display_event_source(disp));
	al_register_event_source(queue, al_get_timer_event_source(timer));

	ALLEGRO_EVENT ev;

	al_start_timer(timer);
	for (;;) {
		al_wait_for_event(queue, &ev);
		switch (ev.type) {
			case ALLEGRO_EVENT_TIMER:
				c8_tick(&c8);
				break;
			case ALLEGRO_EVENT_KEY_DOWN:
			case ALLEGRO_EVENT_KEY_UP:
				c8_key_toggle(&c8, ev.keyboard.keycode);
				break;
			case ALLEGRO_EVENT_DISPLAY_CLOSE:
				c8.quitflag = 1;
				break;
		}
		if (c8.quitflag)
			break;

		c8_execinst(&c8); /* putting everything in main is Bad */

		if (c8.drawflag && al_is_event_queue_empty(queue)) {
			draw_screen(&c8);
			c8.drawflag = 0;
		}
	}

	al_destroy_display(disp);
	al_destroy_timer(timer);
	al_destroy_event_queue(queue);

	return 0;
}
