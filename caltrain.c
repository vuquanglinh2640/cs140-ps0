#include "pintos_thread.h"

struct station {
	int waiting_passengers;
	int empty_seats;
	int on_train_passengers;
	struct condition *cond_get_on_board;
	struct condition *cond_all_passengers_seated;
	struct lock *lck;
};

void
station_init(struct station *station)
{
	station->cond_get_on_board = malloc(sizeof(struct condition));
	station->cond_all_passengers_seated = malloc(sizeof(struct condition));
	station->lck = malloc(sizeof(struct lock));
	cond_init(station->cond_get_on_board);
	cond_init(station->cond_all_passengers_seated);
	lock_init(station->lck);
	station->waiting_passengers = 0;
	station->empty_seats = 0;
	station->on_train_passengers = 0;
}

void
station_load_train(struct station *station, int count)
{
	lock_acquire(station->lck);

	station->empty_seats = count;

	while ((station->empty_seats > 0) && (station->waiting_passengers > 0)) {
		cond_broadcast(station->cond_get_on_board,station->lck);
		cond_wait(station->cond_all_passengers_seated,station->lck);
	}
	station->empty_seats = 0;

	lock_release(station->lck);
}

void
station_wait_for_train(struct station *station)
{
	lock_acquire(station->lck);

	station->waiting_passengers++;

	while (station->on_train_passengers == station->empty_seats)
		cond_wait(station->cond_get_on_board,station->lck);

	station->on_train_passengers++;
	station->waiting_passengers--;

	lock_release(station->lck);
}

void
station_on_board(struct station *station)
{
	lock_acquire(station->lck);

	station->on_train_passengers--;
	station->empty_seats--;

	if ((station->empty_seats == 0) || (station->on_train_passengers == 0))
		cond_signal(station->cond_all_passengers_seated,station->lck);

	lock_release(station->lck);
}
