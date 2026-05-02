/** Contextual Help System - Relies on a local help.json file **/

var helpData = null; // Cached help data from /help.json

function loadHelpData(callback) {
  // Fetch and cache the help.json file
  if (helpData !== null) {
    if (callback) callback();
    return;
  }

  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status >= 200 && this.status < 300) {
      try {
        helpData = JSON.parse(this.responseText);
        if (callback) callback();
      } catch (e) {
        console.error("Failed to parse help.json:", e);
      }
    }
  };
  xhttp.open("GET", "/help.json", true);
  xhttp.send();
}

function showHelpModal(title, text) {
  // Create or show the help modal with the provided text
  var modal = getEl("helpModal");
  if (!modal) {
    // Create modal structure
    modal = document.createElement("div");
    modal.id = "helpModal";
    modal.className = "help-modal";
    modal.innerHTML =
      '<div class="help-modal-content">' +
      '<span class="help-modal-close">&times;</span>' +
      '<h3 id="helpModalTitle"></h3>' +
      '<p id="helpModalText"></p>' +
      "</div>";
    document.body.appendChild(modal);

    // Close button handler
    modal.querySelector(".help-modal-close").onclick = function () {
      modal.style.display = "none";
    };

    // Click outside to close
    modal.onclick = function (event) {
      if (event.target === modal) {
        modal.style.display = "none";
      }
    };
  }

  // Set content and show
  getEl("helpModalTitle").textContent = title || "Help";
  getEl("helpModalText").textContent = text || "No help available.";
  modal.style.display = "block";
}

function createHelpIcon(labelText, helpText) {
  // Create an info icon that shows help when clicked
  var icon = document.createElement("span");
  icon.className = "help-icon";
  icon.innerHTML = "&#9432;"; // Unicode info symbol (i in circle)
  icon.title = "Click for help";
  icon.onclick = function (e) {
    e.preventDefault();
    e.stopPropagation();
    showHelpModal(labelText, helpText);
  };
  return icon;
}

function initializeHelp(section) {
  // Initialize help icons for all fields in the specified section
  loadHelpData(function () {
    if (!helpData || !helpData[section]) {
      return;
    }

    var sectionHelp = helpData[section];
    for (var fieldId in sectionHelp) {
      if (sectionHelp.hasOwnProperty(fieldId)) {
        var element = getEl(fieldId);
        if (element) {
          var helpText = sectionHelp[fieldId];
          
          // Skip if helpText is null or empty (no help available)
          if (!helpText) {
            continue;
          }
          
          // Find the parent setting div or label to insert the icon
          var parent = element.closest(".setting");
          if (parent) {
            // Find the label text
            var label = parent.querySelector("b, .label");
            var labelText = fieldId; // Fallback to field ID
            
            if (label) {
              // Extract and clean the label text
              labelText = label.textContent || label.innerText || fieldId;
              labelText = labelText.trim().replace(/:$/, ""); // Remove trailing colon
              
              // Create and add the help icon
              var helpIcon = createHelpIcon(labelText, helpText);
              label.appendChild(document.createTextNode(" "));
              label.appendChild(helpIcon);
            } else {
              // For toggle switches, find the label span
              var toggleLabel = parent.querySelector("label .label");
              if (toggleLabel) {
                labelText = toggleLabel.textContent || toggleLabel.innerText || fieldId;
                labelText = labelText.trim().replace(/:$/, "");
                
                var helpIcon = createHelpIcon(labelText, helpText);
                toggleLabel.appendChild(document.createTextNode(" "));
                toggleLabel.appendChild(helpIcon);
              }
            }
          }
        }
      }
    }
  });
}
