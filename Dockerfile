# Build stage
FROM gcc:latest as builder

# Install dependencies
RUN apt-get update && apt-get install -y \
    libmicrohttpd-dev \
    make \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source code
COPY . .

# Build the application
RUN make clean && make all

# Runtime stage
FROM debian:bullseye-slim

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libmicrohttpd12 \
    && rm -rf /var/lib/apt/lists/*

# Create non-root user
RUN useradd -m -s /bin/bash appuser

# Set working directory
WORKDIR /app

# Copy binaries from builder
COPY --from=builder /app/bin/indexer /app/bin/indexer
COPY --from=builder /app/bin/search_server /app/bin/search_server

# Create data directory
RUN mkdir -p /app/data && \
    chown -R appuser:appuser /app

# Switch to non-root user
USER appuser

# Expose port
EXPOSE 8080

# Default command (can be overridden)
CMD ["/app/bin/search_server", "index.dat", "8080"] 