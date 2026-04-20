# GitHub Copilot Guide for Beginners

## 🤖 **How Copilot Works in This Project**

### **When You're Editing Files** (Most Common)
- Open any `.cpp`, `.h`, or `.ino` file
- Ask Copilot for changes, suggestions, or new code
- **Copilot automatically knows** (via our instruction files):
  - Your coding standards (variable naming, memory management)
  - Platform constraints (ESP32 vs ATMega2560)
  - Project architecture patterns
  - Security best practices

**Just work normally** - the improvements happen automatically in the background.

### **When You're in Copilot Chat**
- Copilot only sees what you tell it
- For simple help: paste your code and ask questions
- For project context: use `@workspace` so Copilot sees your project

---

## ⚡ **Most Common Commands** (Official VS Code Syntax)

### 🎓 **Get Code Review/Feedback**

**Simple review**:
```
Review this code for embedded systems issues:

[paste your code]
```

**Comprehensive review** (uses our custom prompt file):
```
@workspace Use the file .github/prompts/code-review.md

[paste your code]
```

### 🚀 **Get Something Built**

**Simple task** (Copilot can figure it out):
```
Add a function to blink an LED with configurable timing
```

**Complex task** (needs project context):
```
@workspace Create a WiFi configuration manager that stores credentials in NVS and provides a web interface
```

### 🐛 **Get Help Debugging**

**Quick help**:
```
My ESP32 keeps crashing after a few hours. Help me investigate this systematically.
```

**Guided debugging** (uses our custom prompt file):
```
@workspace Use .github/prompts/debugging-guide.md

My device crashes randomly - where do I start?
```

---

## 🔍 **What Happens Automatically** 

These files work in the background without any commands:

| File | What It Does |
|------|--------------|
| `architecture.md` | Ensures code works on both ESP32 and ATMega2560 |
| `coding-standards.md` | Applies consistent naming and memory patterns |
| `security.md` | Adds embedded security best practices |
| `platformio.md` | Handles build configuration correctly |
| `testing.md` | Suggests proper testing approaches |

**You don't need to know these exist** - they just make Copilot smarter for your project.

---

## 🎯 **When to Use @workspace**

| Task | Need @workspace? | Example |
|------|------------------|---------|
| Review small code snippet | ❌ No | Just paste code in chat |
| Add function to current file | ❌ No | Ask while editing the file |
| Build feature across multiple files | ✅ Yes | `@workspace Create a WiFi manager` |
| Use custom prompt files | ✅ Yes | `@workspace Use .github/prompts/code-review.md` |
| Understand existing code structure | ✅ Yes | `@workspace How does communication work between devices?` |

**Rule of thumb**: If you need Copilot to see other files in your project, use `@workspace`.

---

## 🎛️ **Official VS Code Copilot Features**

### **Chat Participants** (use with @)
- `@workspace` - Include project context
- `@vscode` - VS Code specific help  
- `@terminal` - Terminal and command help

### **Chat Variables** (use with #)
- `#file` - Reference specific files
- `#codebase` - Include entire codebase context
- `#terminalSelection` - Include terminal output

### **Built-in Agents**
- **Agent** - Autonomous implementation across files
- **Plan** - Creates implementation plans before coding
- **Ask** - Q&A without making changes

### **Slash Commands** (Official VS Code)
Type `/` in chat to see available commands (these are VS Code built-ins, not custom shortcuts).

---

## 📋 **Available Custom Prompt Files**

Use these **verified commands** to access our specialized help:

| Command | What It Does |
|---------|--------------|
| `@workspace Use .github/prompts/code-review.md` | Reviews code for embedded systems issues |
| `@workspace Use .github/prompts/debugging-guide.md` | Systematic debugging help |
| `@workspace Use .github/prompts/collaborative-development.md` | Plans solutions with you |
| `@workspace Use .github/prompts/architecture-planning.md` | Designs system architecture |

**Note**: Full file paths are required - VS Code doesn't have shortcuts for custom files.

---

## 📖 **Usage Examples**

### **Basic Code Generation**
**Syntax**: `[natural language description]`  
**Usage**: While editing a file, ask for code additions
```
Example: Add a function to blink an LED with configurable timing
Result: Generates proper embedded C++ with uint16_t, camelCase functions
```

### **Simple Code Review**
**Syntax**: `Review this code for [specific type] issues:`  
**Usage**: Paste code block after the colon for basic analysis
```
Review this code for embedded systems issues:

void setup() {
  Serial.begin(9600);
  String msg = "Hello";
}
```

### **Advanced Code Review with Custom Prompts**
**Syntax**: `@workspace Use [.github/prompts/filename.md]`  
**Usage**: References our specialized prompt files for detailed analysis
```
@workspace Use .github/prompts/code-review.md

[paste your code here]
```

### **Project Context Queries**
**Syntax**: `@workspace [question about your project]`  
**Usage**: Includes your entire codebase in the analysis
```
@workspace How do the ProtonPack and NeutronaWand communicate?
```

### **Multi-file Implementation**
**Syntax**: `@workspace Create [description of feature]`  
**Usage**: Builds features across multiple files with project conventions
```
@workspace Create a WiFi configuration manager with web interface
```

---

## 💡 **Best Practices**

1. **Start simple**: Most of the time, just ask Copilot normally while editing files
2. **Use @workspace for complex tasks**: When you need multiple files or project context
3. **Paste code for reviews**: Copy/paste code snippets into chat for feedback
4. **Trust the automation**: Your instruction files make Copilot smarter automatically
5. **Reference official docs**: VS Code has comprehensive [Copilot documentation](https://code.visualstudio.com/docs/copilot/overview)

---

## 🤔 **Common Questions**

**Q: Do I need to remember those long file paths?**  
A: You can bookmark the four main ones, or just ask Copilot "review this code" for simple cases.

**Q: Are there shortcuts for those file paths?**  
A: No verified shortcuts exist. The full path `@workspace Use .github/prompts/code-review.md` is required.

**Q: What are VS Code's slash commands?**  
A: Type `/` in Copilot Chat to see built-in VS Code commands. These aren't shortcuts to our files.

**Q: Why does my code look different when Copilot suggests it?**  
A: Your custom instruction files automatically apply better naming conventions, memory management, and embedded systems practices.