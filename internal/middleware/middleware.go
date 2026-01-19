package middleware

import (
	"context"
	"net/http"
	"strings"

	"github.com/moh-any/Scrabble-Backend/internal/api"
	"github.com/moh-any/Scrabble-Backend/internal/store"
	"github.com/moh-any/Scrabble-Backend/internal/utils"
)

type UserMiddleware struct {
	UserStore    store.UserStore
	TokenHandler api.TokenHandler
}

type contextKey string

const UserContextKey = contextKey("user")

func SetUser(r *http.Request, user *store.User) *http.Request {
	ctx := context.WithValue(r.Context(), UserContextKey, user)
	return r.WithContext(ctx)
}

func GetUser(r *http.Request) *store.User {
	user, ok := r.Context().Value(UserContextKey).(*store.User)
	if !ok {
		panic("missing user in request")
	}
	return user
}

func (um *UserMiddleware) Authenticate(next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		w.Header().Add("Vary", "Authorization")
		authHeader := r.Header.Get("Authorization")

		if authHeader == "" {
			r = SetUser(r, store.AnonymousUser)
			next.ServeHTTP(w, r)
			return
		}

		headerParts := strings.Split(authHeader, " ")
		if len(headerParts) != 2 || headerParts[0] != "Bearer" {
			utils.WriteJson(w, http.StatusUnauthorized, map[string]any{"error": "invalid authorization header"})
			return
		}

		token := headerParts[1]
		claimsMap, err := um.TokenHandler.ValidateToken(token)
		if err != nil {
			utils.WriteJson(w, http.StatusUnauthorized, map[string]any{"error": "invalid token"})
			return
		}
		userName := claimsMap["username"].(string)
		user, err := um.UserStore.GetUserByUserName(userName)
		if err != nil {
			utils.WriteJson(w, http.StatusUnauthorized, map[string]any{"error": "invalid token"})
			return
		}

		if user == nil {
			utils.WriteJson(w, http.StatusUnauthorized, map[string]any{"error": "token expired or invalid"})
			return
		}

		r = SetUser(r, user)
		next.ServeHTTP(w, r)
	})
}

func (um *UserMiddleware) RequireUser(next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		user := GetUser(r)

		if user.IsAnonymous() {
			utils.WriteJson(w, http.StatusUnauthorized, map[string]any{"error": "you must be logged in to access this route"})
			return
		}

		next.ServeHTTP(w, r)
	})
}
