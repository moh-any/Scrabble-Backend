package app

import (
	"database/sql"
	"fmt"
	"log"
	"net/http"
	"os"

	"github.com/moh-any/Scrabble-Backend/internal/api"
	"github.com/moh-any/Scrabble-Backend/internal/middleware"
	"github.com/moh-any/Scrabble-Backend/internal/store"
	"github.com/moh-any/Scrabble-Backend/migrations"
)

type Application struct {
	Logger       *log.Logger
	UserHandler  *api.UserHandler
	TokenHandler *api.TokenHandler
	Middleware   middleware.UserMiddleware
	DB           *sql.DB
}

func NewApplication() (*Application, error) {
	logger := log.New(os.Stdout, "", log.Ldate|log.Ltime)
	pgdb, err := store.Open()
	if err != nil {
		return nil, err
	}
	err = store.MigrateFS(pgdb, ".", migrations.FS)
	if err != nil {
		panic(err)
	}

	UserStore := store.NewPostgresUserStore(pgdb)

	UserHandler := api.NewUserHandler(UserStore, logger)
	jwtSecret := os.Getenv("JWT_SECRET")
	TokenHandler := api.NewTokenHandler(UserStore, logger, jwtSecret)
	MiddleWareHandler := middleware.UserMiddleware{
		UserStore: UserStore,
	}
	app := Application{
		Logger:       logger,
		UserHandler:  UserHandler,
		TokenHandler: TokenHandler,
		Middleware:   MiddleWareHandler,
		DB:           pgdb,
	}
	return &app, nil
}

func (a *Application) HealthCheck(w http.ResponseWriter, r *http.Request) {
	fmt.Fprintf(w, "Status is available\n")
}
