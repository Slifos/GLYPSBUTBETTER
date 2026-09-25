# GLYPSBUTBETTER

Application où des étudiants créent des activités/événements (tournoi Smash, foot, révision SQL, afterwork…) et rejoignent celles des autres.

## Fonctionnalités

- Créer, modifier, supprimer un événement
- Voir les événements disponibles
- S'inscrire / se désinscrire (places limitées)
- Notification à l'inscription et quand un événement est complet

## Architecture

```
React Client ──REST──▶ API Gateway ──▶ event-service  (Spring Boot, PostgreSQL)
                                   └─▶ user-service   (FastAPI, PostgreSQL)

event-service ──gRPC──▶ statistics-service
(bonus) event-service ──RabbitMQ──▶ notification-service
```

| Dossier | Rôle |
|---|---|
| `api-gateway/` | Routing (Spring Cloud Gateway) |
| `event-service/` | Événements et inscriptions |
| `user-service/` | Utilisateurs |
| `statistics-service/` | Statistiques d'événement via gRPC (participants, places restantes, taux d'occupation) |
| `frontend/` | Interface React |

## Stack

Java 21, Spring Boot, Spring Data JPA (event-service), Python + FastAPI + SQLAlchemy (user-service), PostgreSQL, gRPC, React + Axios, Maven, Docker Compose. Bonus : RabbitMQ.

## API principale (event-service)

| Méthode | Route | Description |
|---|---|---|
| GET | `/events` | Liste des événements |
| GET | `/events/{id}` | Détail |
| POST | `/events` | Création |
| PUT / DELETE | `/events/{id}` | Modification / suppression |
| POST | `/events/{id}/registrations` | Rejoindre |
| DELETE | `/events/{id}/registrations/{userId}` | Quitter |
| GET | `/events/{id}/statistics` | Statistiques détaillées de l'événement (via statistics-service) |
| GET | `/events/statistics/dashboard` | Statistiques cross-événements (via statistics-service) |

Erreurs : `EVENT_NOT_FOUND` (404), `USER_NOT_FOUND` (404), `EVENT_FULL` (400), `ALREADY_REGISTERED` (400), `INVALID_EVENT_DATE` (400).

## Lancer

```
docker compose up
```
