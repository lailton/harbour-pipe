# Harbour PIPE

This project provides a simple server-client communication system using pipes. Below are instructions for setting up the environment, building, and testing the application.

## Environment Setup

To configure the environment, run:

```bash
env.bat
```

## Build

To compile the project, execute:

```bash
build.bat
```

## Testing

### Server

To start the server, run:

```bash
.\bin\server
```

### Multi-Threaded Server

To handle multiple connections, use the multi-threaded server:

```bash
.\bin\server-mt
```

### Client

To connect to the server, run the client. For testing with the multi-threaded server, you can run the client multiple times:

```bash
.\bin\client
```