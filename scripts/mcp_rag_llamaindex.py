#!/usr/bin/env python3
"""
mcp_rag_llamaindex.py
=====================

LlamaIndex-based MCP RAG server for GPStar Proton Pack documentation and source code.

This implementation uses LlamaIndex to:
  - Automatically load and chunk documents from docs/ and source/
  - Generate embeddings on-the-fly
  - Build and persist a vector index
  - Serve queries via MCP protocol

PREREQUISITES
-------------
    pip install llama-index llama-index-embeddings-huggingface

USAGE
-----
    python scripts/mcp_rag_llamaindex.py

The server will:
  1. Check for existing index in extras/rag/
  2. If not found, build from docs/ and source/
  3. Persist for faster subsequent startups
  4. Serve queries via JSON-RPC over stdio

To rebuild the index, delete extras/rag/ and restart.
"""

import json
import sys
from pathlib import Path
from typing import List, Dict, Any, Optional

try:
    from llama_index.core import (
        VectorStoreIndex,
        SimpleDirectoryReader,
        StorageContext,
        load_index_from_storage,
        Settings
    )
    from llama_index.embeddings.huggingface import HuggingFaceEmbedding
except ImportError:
    print("[!] LlamaIndex not installed. Run: pip install llama-index llama-index-embeddings-huggingface", file=sys.stderr)
    sys.exit(1)

# ---------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------

PROJECT_ROOT = Path(__file__).resolve().parent.parent
DOCS_DIR = PROJECT_ROOT / "docs"
SOURCE_DIR = PROJECT_ROOT / "source"
PERSIST_DIR = PROJECT_ROOT / "extras" / "rag"

# Folders/files to exclude from source indexing
EXCLUDE_PATTERNS = [
    ".pio",           # PlatformIO build directory
    ".vscode",        # VSCode settings
    "build",          # Build artifacts
    "lib",            # Third-party libraries
    "test",           # Test files
    ".git",           # Git metadata
    "__pycache__",    # Python cache
    "*.bin",          # Binary files
    "*.hex",          # Hex files
    "*.elf",          # Executable files
    "*.o",            # Object files
    "*.a",            # Archive files
    ".DS_Store",      # macOS metadata
]

# ---------------------------------------------------------------------
# Global state
# ---------------------------------------------------------------------

index: Optional[VectorStoreIndex] = None

# ---------------------------------------------------------------------
# Initialization
# ---------------------------------------------------------------------

def should_exclude(path: Path) -> bool:
    """Check if a path should be excluded based on EXCLUDE_PATTERNS."""
    path_str = str(path)
    for pattern in EXCLUDE_PATTERNS:
        if pattern.startswith("*."):
            # File extension pattern
            if path.name.endswith(pattern[1:]):
                return True
        else:
            # Directory or file name pattern
            if pattern in path.parts or path.name == pattern:
                return True
    return False

def load_documents() -> List:
    """Load documents from docs/ and source/ with filtering."""
    print(f"[+] Loading documents from {DOCS_DIR} and {SOURCE_DIR}...", file=sys.stderr)
    
    all_documents = []
    
    # Load documentation (typically markdown, txt files)
    if DOCS_DIR.exists():
        try:
            docs_reader = SimpleDirectoryReader(
                input_dir=str(DOCS_DIR),
                recursive=True,
                required_exts=[".md", ".txt", ".rst"]
            )
            docs = docs_reader.load_data()
            all_documents.extend(docs)
            print(f"[+] Loaded {len(docs)} documents from docs/", file=sys.stderr)
        except Exception as e:
            print(f"[!] Error loading docs: {e}", file=sys.stderr)
    
    # Load source code with filtering
    if SOURCE_DIR.exists():
        try:
            # Collect source files manually to apply filtering
            source_files = []
            for ext in [".h", ".hpp", ".cpp", ".c", ".js", ".html", ".css", ".py", ".ino"]:
                for file_path in SOURCE_DIR.rglob(f"*{ext}"):
                    if not should_exclude(file_path):
                        source_files.append(str(file_path))
            
            if source_files:
                source_reader = SimpleDirectoryReader(
                    input_files=source_files
                )
                sources = source_reader.load_data()
                all_documents.extend(sources)
                print(f"[+] Loaded {len(sources)} source files from source/", file=sys.stderr)
        except Exception as e:
            print(f"[!] Error loading source files: {e}", file=sys.stderr)
    
    print(f"[+] Total documents loaded: {len(all_documents)}", file=sys.stderr)
    return all_documents

def initialize_index() -> Optional[VectorStoreIndex]:
    """Initialize or load the vector index."""
    global index
    
    # Configure LlamaIndex to use lightweight local embeddings
    # Disable LLM to avoid requiring OpenAI API key
    Settings.llm = None  # No LLM synthesis, just return chunks
    Settings.embed_model = HuggingFaceEmbedding(
        model_name="all-MiniLM-L6-v2"
    )
    Settings.chunk_size = 512
    Settings.chunk_overlap = 50
    
    # Try to load existing index
    if PERSIST_DIR.exists():
        try:
            print(f"[+] Loading existing index from {PERSIST_DIR}...", file=sys.stderr)
            storage_context = StorageContext.from_defaults(persist_dir=str(PERSIST_DIR))
            index = load_index_from_storage(storage_context)
            print(f"[+] Index loaded successfully", file=sys.stderr)
            return index
        except Exception as e:
            print(f"[!] Error loading index: {e}", file=sys.stderr)
            print(f"[+] Will rebuild index...", file=sys.stderr)
    
    # Build new index
    try:
        print(f"[+] Building new index (this may take a few minutes)...", file=sys.stderr)
        documents = load_documents()
        
        if not documents:
            print(f"[!] No documents found to index", file=sys.stderr)
            return None
        
        index = VectorStoreIndex.from_documents(documents, show_progress=True)
        
        # Persist for future use
        PERSIST_DIR.mkdir(parents=True, exist_ok=True)
        index.storage_context.persist(persist_dir=str(PERSIST_DIR))
        print(f"[+] Index built and persisted to {PERSIST_DIR}", file=sys.stderr)
        
        return index
    except Exception as e:
        print(f"[!] Error building index: {e}", file=sys.stderr)
        import traceback
        traceback.print_exc(file=sys.stderr)
        return None

# ---------------------------------------------------------------------
# JSON-RPC MCP Implementation
# ---------------------------------------------------------------------

def create_response(request_id: Any, result: Any = None, error: Any = None) -> Dict:
    """Create a JSON-RPC response."""
    response = {
        "jsonrpc": "2.0",
        "id": request_id
    }
    if error:
        response["error"] = error
    else:
        response["result"] = result
    return response

def create_error(code: int, message: str, data: Any = None) -> Dict:
    """Create a JSON-RPC error object."""
    error = {"code": code, "message": message}
    if data:
        error["data"] = data
    return error

def query_rag(query: str, top_k: int = 5) -> Dict:
    """Query the RAG system using LlamaIndex."""
    if not query:
        return create_error(-32602, "Invalid params: query is required")
    
    if index is None:
        return create_error(-32000, "RAG index not initialized")
    
    try:
        top_k = min(max(top_k, 1), 20)  # Clamp between 1-20
        
        # Use retriever instead of query engine to avoid LLM requirement
        retriever = index.as_retriever(similarity_top_k=top_k)
        nodes = retriever.retrieve(query)
        
        # Format results
        results = []
        for node in nodes:
            # Extract metadata
            file_path = node.node.metadata.get("file_path", "unknown")
            # Make path relative to project root
            try:
                file_path = str(Path(file_path).relative_to(PROJECT_ROOT))
            except ValueError:
                pass
            
            results.append({
                "file": file_path,
                "content": node.node.get_content(),
                "score": float(node.score) if node.score else 0.0,
                "metadata": {
                    k: str(v) for k, v in node.node.metadata.items()
                    if k not in ["file_path"]  # Avoid duplication
                }
            })
        
        return {
            "results": results
        }
    except Exception as e:
        import traceback
        traceback.print_exc(file=sys.stderr)
        return create_error(-32603, f"Internal error: {str(e)}")

def handle_request(request: Dict) -> Dict:
    """Handle a single JSON-RPC request."""
    request_id = request.get("id")
    method = request.get("method")
    params = request.get("params", {})
    
    if method == "initialize":
        return create_response(request_id, {
            "capabilities": {
                "tools": {
                    "listChanged": False
                }
            },
            "serverInfo": {
                "name": "gpstar-rag-llamaindex",
                "version": "1.0.0"
            }
        })
    
    elif method == "tools/list":
        return create_response(request_id, {
            "tools": [
                {
                    "name": "query_rag",
                    "description": "Query GPStar Proton Pack documentation and source code using semantic similarity (powered by LlamaIndex)",
                    "inputSchema": {
                        "type": "object",
                        "properties": {
                            "query": {
                                "type": "string",
                                "description": "Search query"
                            },
                            "top_k": {
                                "type": "integer",
                                "description": "Number of results (1-20)",
                                "default": 5,
                                "minimum": 1,
                                "maximum": 20
                            }
                        },
                        "required": ["query"]
                    }
                }
            ]
        })
    
    elif method == "tools/call":
        tool_name = params.get("name")
        arguments = params.get("arguments", {})
        
        if tool_name == "query_rag":
            result = query_rag(
                arguments.get("query", ""),
                arguments.get("top_k", 5)
            )
            if "code" in result:  # Error response
                return create_response(request_id, None, result)
            else:
                return create_response(request_id, {
                    "content": [
                        {
                            "type": "text",
                            "text": json.dumps(result, indent=2)
                        }
                    ]
                })
        else:
            return create_response(request_id, None, create_error(-32601, f"Unknown tool: {tool_name}"))
    
    else:
        return create_response(request_id, None, create_error(-32601, f"Method not found: {method}"))

# ---------------------------------------------------------------------
# Main JSON-RPC server loop
# ---------------------------------------------------------------------

def main():
    """Main MCP server loop using JSON-RPC over stdio."""
    global index
    
    # Initialize index
    print("[+] Initializing LlamaIndex RAG system...", file=sys.stderr)
    index = initialize_index()
    
    if index is None:
        print("[!] Failed to initialize RAG index. Exiting.", file=sys.stderr)
        sys.exit(1)
    
    print("[+] RAG system initialized successfully", file=sys.stderr)
    print("[+] Starting MCP server (JSON-RPC over stdio)...", file=sys.stderr)
    
    # Read from stdin and write to stdout (JSON-RPC over stdio)
    while True:
        try:
            line = sys.stdin.readline()
            if not line:
                break
                
            line = line.strip()
            if not line:
                continue
                
            # Parse JSON-RPC request
            try:
                request = json.loads(line)
            except json.JSONDecodeError as e:
                error_response = create_response(None, None, create_error(-32700, "Parse error"))
                print(json.dumps(error_response))
                continue
                
            # Handle request
            response = handle_request(request)
            print(json.dumps(response))
            sys.stdout.flush()
            
        except KeyboardInterrupt:
            break
        except Exception as e:
            print(f"[!] Unexpected error: {e}", file=sys.stderr)
            import traceback
            traceback.print_exc(file=sys.stderr)
            break

if __name__ == "__main__":
    main()
