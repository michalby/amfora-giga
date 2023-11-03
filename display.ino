void displayTest() {
  // Clear the buffer
  display.clearBuffer();

  // Draw a black rectangle
  int rectWidth = 250; // Adjust this based on your text length
  int rectHeight = 30; // Adjust this based on your font size
  int rectX = (display.width() - rectWidth) / 2;
  int rectY = 10;
  display.fillRect(rectX, rectY, rectWidth, rectHeight, EPD_BLACK);

  // Display white text on top of the rectangle
  display.setTextColor(EPD_WHITE);
  display.setFont(&FreeMonoBold18pt7b); // Use a bold font with larger size
  String text = "Hello, World!";
  int textWidth = text.length() * 12; // Approximate width, adjust based on font
  display.setCursor((display.width() - textWidth) / 2, rectY + 25);  // Adjust the Y value based on font metrics
  display.print(text);

  // Display the date on the top right corner
  display.setFont(&FreeMono9pt7b); // Use a smaller font for the date
  display.setTextColor(EPD_BLACK);
  String date = "25/10/2023"; // Placeholder date, replace with RTC value if available
  display.setCursor(display.width() - 80, 20); // Adjust based on your display and font size
  display.print(date);
  
  // Refresh the display to show the text and rectangle
  display.display();
}