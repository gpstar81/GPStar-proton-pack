---
description: 'Security best practices for embedded IoT devices including ESP32 WiFi security, NVS encryption, and secure communication patterns'
applyTo: '**/*.{cpp,h,hpp,ino}'
---

# Embedded Systems Security Guidelines

## WiFi and Network Security

### Secure Connection Patterns
```cpp
// Use WPA3 when available, fallback to WPA2
void initSecureWiFi() {
    WiFi.begin(ssid, password);
    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(true);
    
    // Disable WPS and other insecure features
    esp_wifi_wps_disable();
}
```

### TLS/SSL for Web Communication
- Always use HTTPS for web interfaces
- Validate certificate chains when communicating with external services
- Use strong cipher suites and disable deprecated protocols

```cpp
#include <WiFiClientSecure.h>

WiFiClientSecure client;
// Set root CA certificate
client.setCACert(root_ca);
// Verify server identity
client.setInsecure(false);  // Don't skip verification
```

### Access Point Security
```cpp
// When creating AP, use strong encryption
WiFi.softAP(ap_ssid, ap_password, channel, hidden, max_connections);
WiFi.softAPConfig(local_IP, gateway, subnet);

// Disable unnecessary services
WiFi.softAPdisconnect(false);  // Keep AP config
```

## Data Protection and Storage

### NVS (Non-Volatile Storage) Security
- Encrypt sensitive data before storing in NVS
- Use partition encryption for ESP32
- Implement secure key derivation

```cpp
#include "nvs_flash.h"
#include "esp_partition.h"

// Initialize NVS with encryption
esp_err_t init_nvs_encrypted() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    return ret;
}
```

### Sensitive Data Handling
```cpp
// Use secure string handling
class SecureString {
private:
    char* data;
    size_t length;
public:
    SecureString(const char* input) {
        length = strlen(input);
        data = new char[length + 1];
        memcpy(data, input, length + 1);
    }
    
    ~SecureString() {
        // Zero memory before deallocation
        memset(data, 0, length);
        delete[] data;
    }
};
```

## Authentication and Authorization

### API Key Management
- Never hardcode API keys or passwords in source code
- Store credentials in encrypted NVS partitions
- Implement key rotation mechanisms

```cpp
// Bad: Never do this
// const char* api_key = "sk-1234567890abcdef";

// Good: Load from secure storage
String loadApiKey() {
    Preferences prefs;
    prefs.begin("secure", true);  // Read-only, encrypted
    String key = prefs.getString("api_key", "");
    prefs.end();
    return key;
}
```

### Web Interface Authentication
```cpp
// Implement proper session management
class WebAuth {
private:
    String generateSessionId();
    bool validateSession(String sessionId);
    
public:
    bool authenticate(String username, String password) {
        // Use secure password verification
        // Implement rate limiting
        // Log authentication attempts
    }
};
```

## Input Validation and Sanitization

### Web Input Validation
```cpp
bool validateInput(String input, size_t maxLength, bool allowSpecialChars = false) {
    if (input.length() > maxLength) return false;
    
    // Check for malicious patterns
    if (input.indexOf("<script") >= 0) return false;
    if (input.indexOf("javascript:") >= 0) return false;
    
    if (!allowSpecialChars) {
        // Only allow alphanumeric and safe chars
        for (char c : input) {
            if (!isalnum(c) && c != '_' && c != '-') {
                return false;
            }
        }
    }
    return true;
}
```

### Buffer Overflow Prevention
```cpp
// Use safe string functions
void safeStringCopy(char* dest, const char* src, size_t destSize) {
    strncpy(dest, src, destSize - 1);
    dest[destSize - 1] = '\0';  // Ensure null termination
}

// Validate buffer sizes before operations
bool processCommand(const char* command, size_t commandLength) {
    if (commandLength >= MAX_COMMAND_SIZE) {
        logSecurityEvent("Command too long", commandLength);
        return false;  
    }
    // Process command safely
}
```

## Secure Communication Protocols

### Inter-Device Communication
```cpp
// Use encrypted communication between devices
struct EncryptedMessage {
    uint8_t iv[16];           // Initialization vector
    uint8_t payload[256];     // Encrypted data
    uint8_t hmac[32];        // Message authentication
};

bool sendSecureMessage(const char* data) {
    EncryptedMessage msg;
    
    // Generate random IV
    esp_fill_random(msg.iv, sizeof(msg.iv));
    
    // Encrypt data with AES
    encryptData(data, msg.payload, msg.iv);
    
    // Calculate HMAC for integrity
    calculateHMAC(msg.payload, msg.hmac);
    
    return transmitMessage(&msg);
}
```

### WebSocket Security
```cpp
// Implement secure WebSocket communications
void handleWebSocketMessage(String message) {
    // Validate message format
    if (!validateJSONFormat(message)) {
        closeConnection("Invalid message format");
        return;
    }
    
    // Check message size limits
    if (message.length() > MAX_MESSAGE_SIZE) {
        closeConnection("Message too large");
        return;
    }
    
    // Process message with rate limiting
    if (!checkRateLimit()) {
        closeConnection("Rate limit exceeded");
        return;
    }
}
```

## Firmware Security

### Secure Boot and Updates
```cpp
// Verify firmware signatures before updates
bool validateFirmwareSignature(const uint8_t* firmware, size_t size) {
    // Implement digital signature verification
    // Check against known public keys
    // Verify firmware integrity
    return true;  // Placeholder
}

// Secure OTA update process
void performSecureOTA(const String& firmwareURL) {
    WiFiClientSecure client;
    client.setCACert(ca_cert);
    
    if (client.connect(server, 443)) {
        // Download and verify firmware
        // Validate signature before flashing
        // Implement rollback on failure
    }
}
```

### Memory Protection Techniques
- Enable stack canaries where available
- Use memory MPU (Memory Protection Unit) features
- Implement heap corruption detection

```cpp
// Memory corruption detection
void* safeMalloc(size_t size) {
    void* ptr = malloc(size);
    if (ptr) {
        // Add canary values around allocation
        addMemoryCanary(ptr, size);
    }
    return ptr;
}

void safeFree(void* ptr) {
    if (validateMemoryCanary(ptr)) {
        free(ptr);
    } else {
        logSecurityEvent("Memory corruption detected");
        // Handle security incident
    }
}
```

## Logging and Monitoring

### Security Event Logging
```cpp
enum SecurityEventType {
    INVALID_INPUT,
    AUTHENTICATION_FAILURE,
    RATE_LIMIT_EXCEEDED,
    MEMORY_CORRUPTION,
    NETWORK_ANOMALY
};

void logSecurityEvent(SecurityEventType type, const char* details) {
    // Log to secure storage
    // Include timestamp, event type, and context
    // Implement log rotation to prevent storage exhaustion
    
    Serial.printf("[SECURITY] Type: %d, Details: %s\n", type, details);
    
    // Consider alerting mechanisms for critical events
    if (type == MEMORY_CORRUPTION) {
        triggerSecurityAlert();
    }
}
```

### Rate Limiting and DDoS Protection
```cpp
class RateLimiter {
private:
    std::map<String, uint32_t> requestCounts;
    uint32_t lastReset;
    
public:
    bool checkLimit(String clientIP, uint32_t maxRequests = 100, uint32_t windowMs = 60000) {
        uint32_t now = millis();
        
        // Reset window
        if (now - lastReset > windowMs) {
            requestCounts.clear();
            lastReset = now;
        }
        
        // Check and update count
        requestCounts[clientIP]++;
        return requestCounts[clientIP] <= maxRequests;
    }
};
```

## Hardware Security Features

### ESP32-Specific Security Features
```cpp
void enableESP32Security() {
    // Enable flash encryption
    esp_flash_encryption_init_checks();
    
    // Enable secure boot
    esp_secure_boot_init_checks();
    
    // Configure eFuse for one-time programmable security
    // (Only in production builds)
    #ifdef PRODUCTION_BUILD
    esp_efuse_burn_key(EFUSE_BLK_KEY0, key_data, sizeof(key_data));
    #endif
}
```

### ATMega2560 Security Considerations
- Lock fuse bits in production to prevent firmware reading
- Use secure bootloaders where available
- Implement software-based security measures for hardware limitations

## Security Testing and Validation

### Penetration Testing Checklist
- [ ] Test for buffer overflows in all input handlers
- [ ] Verify authentication bypass attempts fail
- [ ] Validate rate limiting effectiveness
- [ ] Check for information disclosure in error messages
- [ ] Test firmware update process security
- [ ] Verify secure storage encryption

### Security Code Review
- Review all network communication code for TLS/SSL usage
- Validate input sanitization in web interfaces
- Check for hardcoded credentials or keys
- Verify proper error handling doesn't leak information
- Ensure secure random number generation usage