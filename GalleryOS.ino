/*
  Gallery OS - Linux-like Shell Environment for Arduino R4
  Target Hardware: Arduino Uno R4 (Minima/WiFi), Arduino Nano R4
   
  Features:
  - Bootstrap sequence
  - Command Line Interface (CLI)
  - Simulated Firewall
  - In-Memory Password Manager
  - Mock Backup System
   
  Note: This is a bare-metal firmware that mimics Linux behavior.
  It does not use a Linux kernel due to hardware constraints (No MMU, limited RAM).
*/

#include <Arduino.h>

// --- Configuration ---
#define HOSTNAME "gallery"
#define USER "admin"
#define BAUD_RATE 115200
#define MAX_PASSWORDS 10
#define MAX_BLOCKED_IPS 10

// --- Data Structures ---

struct PasswordEntry {
  String site;
  String username;
  String password;
  bool active;
};

struct FirewallRule {
  String ip;
  bool active;
};

// --- Globals ---
PasswordEntry passwordStore[MAX_PASSWORDS];
FirewallRule blockedIPs[MAX_BLOCKED_IPS];
bool isLoggedIn = false;
String currentDir = "/home/admin";

// --- Function Prototypes ---
void runBootstrap();
void displayPrompt();
void processCommand(String input);
void cmdHelp();
void cmdFirewall(String args);
void cmdPassMan(String args);
void cmdBackup();
void cmdClear();
void cmdUptime();

void setup() {
  Serial.begin(BAUD_RATE);
  while (!Serial) {
    ; // Wait for serial port to connect
  }
   
  // Initialize data
  blockedIPs[0] = {"192.168.1.55", true}; // Default rule
   
  runBootstrap();
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.length() > 0) {
      Serial.println(input); // Echo command
      processCommand(input);
      displayPrompt();
    }
  }
}

// --- Bootstrap System ---
void runBootstrap() {
  Serial.println("\033[2J\033[H"); // ANSI Clear Screen
  Serial.println(F("Bootsrap Loader v1.0.4"));
  delay(500);
  Serial.print(F("Loading Kernel modules..."));
  delay(800);
  Serial.println(F(" [OK]"));
  Serial.print(F("Mounting filesystem..."));
  delay(600);
  Serial.println(F(" [OK]"));
  Serial.print(F("Starting Firewall Service..."));
  delay(400);
  Serial.println(F(" [OK]"));
  Serial.print(F("Initializing Password Vault..."));
  delay(400);
  Serial.println(F(" [OK]"));
   
  Serial.println(F("\nWelcome to Gallery OS (GNU/Arduino)"));
  Serial.println(F("System is ready. Type 'help' for available commands.\n"));
   
  displayPrompt();
}

void displayPrompt() {
  Serial.print(USER);
  Serial.print("@");
  Serial.print(HOSTNAME);
  Serial.print(":");
  Serial.print(currentDir);
  Serial.print("$ ");
}

// --- Terminal / Command Parser ---
void processCommand(String input) {
  int spaceIndex = input.indexOf(' ');
  String cmd = (spaceIndex == -1) ? input : input.substring(0, spaceIndex);
  String args = (spaceIndex == -1) ? "" : input.substring(spaceIndex + 1);

  if (cmd == "help") cmdHelp();
  else if (cmd == "clear") cmdClear();
  else if (cmd == "fw") cmdFirewall(args);
  else if (cmd == "pm") cmdPassMan(args);
  else if (cmd == "backup") cmdBackup();
  else if (cmd == "uptime") cmdUptime();
  else if (cmd == "ls") Serial.println(F("passwords.db  config.sys  firewall.rules"));
  else if (cmd == "reboot") { Serial.println(F("Rebooting...")); delay(1000); runBootstrap(); }
  else {
    Serial.print(F("gallery: command not found: "));
    Serial.println(cmd);
  }
}

void cmdHelp() {
  Serial.println(F("GNU/Arduino Gallery OS Commands:"));
  Serial.println(F("  fw [status|block <ip>]   : Manage Firewall"));
  Serial.println(F("  pm [list|add <site> <u/p>] : Password Manager"));
  Serial.println(F("  backup                   : Run system backup"));
  Serial.println(F("  uptime                   : System uptime"));
  Serial.println(F("  clear                    : Clear terminal"));
  Serial.println(F("  reboot                   : Restart system"));
}

// --- Firewall Module ---
void cmdFirewall(String args) {
  if (args == "" || args == "status") {
    Serial.println(F("--- Firewall Status: ACTIVE ---"));
    Serial.println(F("Policy: ALLOW ALL incoming"));
    Serial.println(F("Blocked IPs:"));
    bool found = false;
    for (int i = 0; i < MAX_BLOCKED_IPS; i++) {
      if (blockedIPs[i].active) {
        Serial.print(F("  [DROP] "));
        Serial.println(blockedIPs[i].ip);
        found = true;
      }
    }
    if (!found) Serial.println(F("  (None)"));
  } else if (args.startsWith("block ")) {
    String ip = args.substring(6);
    for (int i = 0; i < MAX_BLOCKED_IPS; i++) {
      if (!blockedIPs[i].active) {
        blockedIPs[i].ip = ip;
        blockedIPs[i].active = true;
        Serial.print(F("Firewall: Added rule to DROP "));
        Serial.println(ip);
        return;
      }
    }
    Serial.println(F("Firewall: Rule table full."));
  } else {
    Serial.println(F("Usage: fw [status | block <ip>]"));
  }
}

// --- Password Manager Module ---
void cmdPassMan(String args) {
  if (args == "" || args == "list") {
    Serial.println(F("--- Password Vault ---"));
    bool found = false;
    for (int i = 0; i < MAX_PASSWORDS; i++) {
      if (passwordStore[i].active) {
        Serial.print(i);
        Serial.print(F(": "));
        Serial.print(passwordStore[i].site);
        Serial.print(F(" | "));
        Serial.println("********"); // Security feature: don't show pass on list
        found = true;
      }
    }
    if (!found) Serial.println(F("  (Vault empty)"));
  } else if (args.startsWith("add ")) {
    // Expected format: pm add google myUser myPass
    int firstSpace = args.indexOf(' ');
    int secondSpace = args.indexOf(' ', firstSpace + 1);
    int thirdSpace = args.indexOf(' ', secondSpace + 1);
     
    if (firstSpace == -1 || secondSpace == -1 || thirdSpace == -1) {
       Serial.println(F("Usage: pm add <site> <user> <pass>"));
       return;
    }

    String site = args.substring(4, secondSpace);
    String user = args.substring(secondSpace + 1, thirdSpace);
    String pass = args.substring(thirdSpace + 1);

    for (int i = 0; i < MAX_PASSWORDS; i++) {
      if (!passwordStore[i].active) {
        passwordStore[i].site = site;
        passwordStore[i].username = user;
        passwordStore[i].password = pass;
        passwordStore[i].active = true;
        Serial.print(F("PassMan: Credentials saved for "));
        Serial.println(site);
        return;
      }
    }
    Serial.println(F("PassMan: Storage full."));
  } else {
    Serial.println(F("Usage: pm [list | add <site> <user> <pass>]"));
  }
}

// --- Backup System ---
void cmdBackup() {
  Serial.println(F("Starting System Backup..."));
  Serial.println(F("[....................] 0%"));
  delay(300);
  Serial.println(F("[#####...............] 25% - Compressing logs"));
  delay(500);
  Serial.println(F("[##########..........] 50% - Encrypting vault"));
  delay(400);
  Serial.println(F("[###############.....] 75% - Uploading to serial"));
  delay(300);
  Serial.println(F("[####################] 100%"));
  Serial.println(F("Backup Complete. Snapshot hash: 8f4a2c91"));
  Serial.println(F("Backup saved to local storage simulation."));
}

void cmdClear() {
  Serial.println("\033[2J\033[H");
}

void cmdUptime() {
  unsigned long runMillis = millis();
  unsigned long allSeconds = runMillis / 1000;
  int runHours = allSeconds / 3600;
  int runMinutes = (allSeconds % 3600) / 60;
  int runSeconds = allSeconds % 60;

  Serial.print(F("up "));
  Serial.print(runHours);
  Serial.print(F(":"));
  if (runMinutes < 10) Serial.print("0");
  Serial.print(runMinutes);
  Serial.print(F(":"));
  if (runSeconds < 10) Serial.print("0");
  Serial.println(runSeconds);
}
    