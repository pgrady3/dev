#
# This is the user-interface definition of a Shiny web application. You can
# run the application by clicking 'Run App' above.
#
# Find out more about building applications with Shiny here:
# 
#    http://shiny.rstudio.com/
#

library(shiny)

# Define UI for application that draws a histogram
shinyUI(fluidPage(
  # Application title
  titlePanel("DEV Racing Vehicle Watching System"),

  # Sidebar with a slider input for number of bins 
  sidebarLayout(
    sidebarPanel(
       selectInput("variables1", "Variable to Visualize:", 
                   choices = c("Comprehensive", "Speed/V/mph", "Power/UI/w", "Current/I/A", "Voltage/U/V", "Energy Usage/%", "Map"),
                   selected = "Comprehensive"),
       textInput("url", "Server Name", "http://yunfan.colab.duke.edu:5000/datarequest/"),
       textInput("battery","Energy Storage of Battery", "4000"),
       sliderInput("timeLength",
                   "Time Length Displayed:",
                   min = 1, max = 200, value = 75)
    ),
    
    # Show a plot of the generated distribution
    mainPanel(
       plotOutput("distPlot"),
       tableOutput("distTable")
       )
  )
))
