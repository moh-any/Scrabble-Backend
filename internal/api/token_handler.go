package api

import (
	"database/sql"
	"encoding/json"
	"errors"
	"log"
	"net/http"
	"time"

	"github.com/golang-jwt/jwt/v4"
	"github.com/moh-any/Scrabble-Backend/internal/store"
	"github.com/moh-any/Scrabble-Backend/internal/utils"
)

const ScopeAuth = "authentication"

type TokenHandler struct {
	userStore store.UserStore
	logger    *log.Logger
	jwtSecret []byte
}

func NewTokenHandler(userStore store.UserStore, logger *log.Logger, jwtSecret string) *TokenHandler {
	return &TokenHandler{
		userStore: userStore,
		logger:    logger,
		jwtSecret: []byte(jwtSecret),
	}
}

type createTokenRequest struct {
	Username string `json:"username"`
	Password string `json:"password"`
}

func (h *TokenHandler) HandleCreateToken(w http.ResponseWriter, r *http.Request) {
	var req createTokenRequest
	err := json.NewDecoder(r.Body).Decode(&req)

	if err != nil {
		h.logger.Printf("ERROR: createTokenRequest: %v", err)
		utils.WriteJson(w, http.StatusBadRequest, map[string]any{"error": "invalid request payload"})
		return
	}

	user, err := h.userStore.GetUserByUserName(req.Username)
	if err == sql.ErrNoRows {
		h.logger.Printf("Erro: GetUserByUsername: %v", err)
		utils.WriteJson(w, http.StatusBadRequest, map[string]any{"error": "invalid user credintials"})
		return
	}
	if err != nil {
		h.logger.Printf("ERROR: GetUserByUsername: %v", err)
		utils.WriteJson(w, http.StatusInternalServerError, map[string]any{"error": "internal server error"})
		return
	}

	passwordsDoMatch, err := user.PasswordHash.Matches(req.Password)
	if err != nil {
		h.logger.Printf("ERORR: PasswordHash.Mathes %v", err)
		utils.WriteJson(w, http.StatusInternalServerError, map[string]any{"error": "internal server error"})
		return
	}

	if !passwordsDoMatch {
		utils.WriteJson(w, http.StatusUnauthorized, map[string]any{"error": "invalid credentials"})
		return
	}

	token, err := h.GenerateAccessToken(user, 24*time.Hour)
	if err != nil {
		h.logger.Printf("ERORR: Creating Token %v", err)
		utils.WriteJson(w, http.StatusInternalServerError, map[string]any{"error": "internal server error"})
		return

	}

	utils.WriteJson(w, http.StatusCreated, map[string]any{"auth_token": token})

}

func (h *TokenHandler) GenerateAccessToken(user *store.User, ttl time.Duration) (string, error) {
	expirationTime := time.Now().Add(ttl)

	claims := jwt.MapClaims{
		"id":       user.ID,
		"username": user.UserName,
		"email":    user.Email,
		"exp":      expirationTime.Unix(),
		"iat":      time.Now().Unix(),
	}

	token := jwt.NewWithClaims(jwt.SigningMethodHS256, claims)

	tokenString, err := token.SignedString(h.jwtSecret)
	if err != nil {
		return "", err
	}
	return tokenString, nil
}

func (h *TokenHandler) ValidateToken(tokenString string) (jwt.MapClaims, error) {

	token, err := jwt.Parse(tokenString, func(token *jwt.Token) (interface{}, error) {
		_, ok := token.Method.(*jwt.SigningMethodHMAC)
		if !ok {
			return nil, errors.New("invalid token")
		}
		return h.jwtSecret, nil
	})
	if err != nil {
		if errors.Is(err, jwt.ErrTokenExpired) {
			return nil, errors.New("expired token")
		}
		return nil, errors.New("invalid token")
	}

	claims, ok := token.Claims.(jwt.MapClaims)
	if ok && token.Valid {
		return claims, nil
	}
	return nil, errors.New("invalid token")
}
