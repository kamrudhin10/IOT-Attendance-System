function doGet(e) {
  // Check if the event object 'e' is defined
  if (!e) {
    return ContentService.createTextOutput('Error: No event object available.');
  }

  Logger.log(JSON.stringify(e)); // Log the entire event object for debugging
  var result = 'Ok'; // Default result message

  // Check if parameters are present
  if (!e.parameter || Object.keys(e.parameter).length === 0) {
    result = 'No Parameters'; // Set result if no parameters are provided
  } else {
    var sheet_id = '1-r-5c-ZGRYjF2h-tEVgZOy43HkWYsHxQ5eawZD_24NY'; // Spreadsheet ID
    var sheet = SpreadsheetApp.openById(sheet_id).getActiveSheet(); // Access the spreadsheet
    var newRow = sheet.getLastRow() + 1; // Find the next empty row
    var rowData = [];

    // Add current date and time to the rowData array
    var currDate = new Date();
    rowData[0] = currDate; // Date in column A
    rowData[1] = Utilities.formatDate(currDate, "Asia/Kolkata", 'HH:mm:ss'); // Time in column B

    // Loop through the parameters and populate the rowData array
    for (var param in e.parameter) {
      Logger.log('Processing parameter: ' + param);
      var value = stripQuotes(e.parameter[param]); // Clean value from quotes
      Logger.log(param + ': ' + value);

      switch (param) {
        case 'name':
          rowData[2] = value; // Employee Name in column C
          result = 'Employee Name written in column C'; 
          break;
        default:
          result = "Unsupported parameter";
      }
    }

    Logger.log(JSON.stringify(rowData)); // Log the row data being written to the sheet
    var newRange = sheet.getRange(newRow, 1, 1, rowData.length); // Define the range for the new row
    newRange.setValues([rowData]); // Write data to the sheet
  }

  // Return the result as text output
  return ContentService.createTextOutput(result);
}

// Helper function to strip quotes from values
function stripQuotes(value) {
  return value.replace(/^["']|['"]$/g, "");
}
