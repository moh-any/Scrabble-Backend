package main

import (
	"fmt"
	"net/http"
	"os"
	"strconv"
	"time"

	"github.com/joho/godotenv"
	"github.com/moh-any/Scrabble-Backend/internal/app"
	"github.com/moh-any/Scrabble-Backend/internal/routes"
)

func main() {
	godotenv.Load()
	portString := os.Getenv("API_PORT")
	port, err := strconv.ParseInt(portString, 10, 64)
	app, err := app.NewApplication()
	if err != nil {
		panic(err)
	} else {
		app.Logger.Output(1, "Application started")
	}
	defer app.DB.Close()
	server := &http.Server{
		Addr:         fmt.Sprintf(":%d", port),
		Handler:      routes.SetupRoutes(app),
		ReadTimeout:  10 * time.Second,
		WriteTimeout: 30 * time.Second,
		IdleTimeout:  10 * time.Minute,
	}
	app.Logger.Printf("Starting Server at port %d\n", port)
	err = server.ListenAndServe()
	if err != nil {
		app.Logger.Fatal("Failed to Initialize server")
	}
}
