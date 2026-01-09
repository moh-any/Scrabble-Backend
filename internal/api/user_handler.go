package api

import (
	"encoding/json"
	"errors"
	"log"
	"net/http"
	"regexp"

	"github.com/moh-any/Scrabble-Backend/internal/store"
	"github.com/moh-any/Scrabble-Backend/internal/utils"
)

type registerUserRequest struct {
	Username string `json:"username"`
	Email    string `json:"email"`
	Password string `json:"password"`
	Bio      string `json:"bio"`
}

type UserHandler struct {
	userStore store.UserStore
	logger    *log.Logger
}

func NewUserHandler(userStore store.UserStore, logger *log.Logger) *UserHandler {
	return &UserHandler{
		userStore,
		logger,
	}
}

func (h *UserHandler) validateRegisterRequest(req *registerUserRequest) error {
	if req.Username == "" {
		return errors.New("username is required")
	}

	if len(req.Username) > 50 {
		return errors.New("username cannot be greater than 50 characters")
	}

	if req.Email == "" {
		return errors.New("email is required")
	}

	emailRegex := regexp.MustCompile(`^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$`)
	if !emailRegex.MatchString(req.Email) {
		return errors.New("invalid email format")
	}

	// we can have password validatoin aswell
	if req.Password == "" {
		return errors.New("password is required")
	}

	return nil
}

func (uh *UserHandler) HandleRegisterUser(w http.ResponseWriter, r *http.Request) {
	var req registerUserRequest
	err := json.NewDecoder(r.Body).Decode(&req)
	if err != nil {
		uh.logger.Printf("Error: Register Request decoding: %v", err)
		utils.WriteJson(w, http.StatusBadRequest, map[string]any{"error": "Invalid request format"})
		return
	}

	err = uh.validateRegisterRequest(&req)
	if err != nil {
		uh.logger.Printf("Error: Failed validate register request: %v", err)
		utils.WriteJson(w, http.StatusBadRequest, map[string]any{"error": "Invalid request format"})
		return
	}

	user := &store.User{
		UserName: req.Username,
		Email:    req.Email,
	}
	if req.Bio != "" {
		user.Bio = req.Bio
	}

	err = user.PasswordHash.Set(req.Password)
	if err != nil {
		uh.logger.Printf("Error: hashing password %v", err)
		utils.WriteJson(w, http.StatusInternalServerError, map[string]any{"error": "Internal server error"})
		return
	}

	err = uh.userStore.CreateUser(user)
	if err != nil {
		uh.logger.Printf("Error: Failed CreateUser: %v", err)
		utils.WriteJson(w, http.StatusInternalServerError, map[string]any{"error": "Internal Server error"})
		return
	}
	utils.WriteJson(w, http.StatusCreated, map[string]any{"user": user})
}
