#
# This is the server logic of a Shiny web application. You can run the 
# application by clicking 'Run App' above.
#
# Find out more about building applications with Shiny here:
# 
#    http://shiny.rstudio.com/
#

library(shiny)
library(ggplot2)
library(dplyr)
library(gridExtra)
library(jsonlite)
library(httr)
library(maps)
library(leaflet)
library(htmlwidgets)
library(shinyjs)
dataRequest <- function(jsonGetURL, battery){
  if(is.null(jsonGetURL) || is.null(battery)){
    print("invalid input")
    return(NULL)
  }
  r = GET(jsonGetURL, accept_json())
  if(content(r, "text", encoding = "ISO-8859-1") == "No data available"){
    return(NULL)
  } else{
    bin <- fromJSON(content(r, "text", encoding = "ISO-8859-1"))
    data = data.frame(sessionName = bin$sessionName,
                      timeStamp = bin$timeStamp,
                      longtitude = as.double(bin$longitude),
                      latitude = as.double(bin$latitude),
                      voltage = as.double(bin$voltage),
                      current = as.double(bin$current),
                      speed = as.double(bin$speed),
                      mileage = as.double(bin$mileage),
                      heading = as.double(bin$heading),
                      msSinceStart = as.double(bin$msSinceStart),
                      power = as.double(bin$voltage)*as.double(bin$current), 
                      avgPower = battery*as.double(bin$energyUsed)/as.double(bin$msSinceStart),
                      energyUsed = as.double(bin$energyUsed),
                      energyLeft = 1-as.double(bin$energyUsed),
                      bateryStatus = bin$batteryStatus)
    return(data)
  }
}

# Define server logic required to draw a histogram
shinyServer(function(input, output) {
  # generate updating order and prepare variables
  autoInvalidate <- reactiveTimer(500)
  
  output$distPlot <- renderPlot({
    # auto updater
    autoInvalidate()

    # json version
    all <- read.csv("allData.csv")
    url = input$url
    battery = as.integer(input$battery)
    newdata = dataRequest(url, battery)
    if(!is.null(newdata)){
      all <- rbind(all, newdata)
      write.csv(all, file = "allData.csv", row.names = FALSE)
    }
    
    # plot out everything
    theData = tail(all,input$timeLength)
    if(input$variables1 == "Speed/V/mph"){
      a = theData$speed
      b = theData$msSinceStart
      #ylim=range(10, 40),
      plot(b,a, type = "o", col = "black", ylim=range(0, 30),
           xlab = "Time/ms", ylab = "Speed/mph",
           main = "Graph of Speed")
      abline(h = 15, col = "red", lwd = 5)
      lines(lowess(a~b), col="blue", lwd=5)
    } else if(input$variables1 == "Current/I/A"){
      a = theData$current
      b = theData$msSinceStart
      plot(b,a, type = "o", col = "black", ylim=range(0, 3),
           xlab = "Time/ms", ylab = "Current/A",
           main = "Graph of Current")
      abline(h = 1.5, col = "red", lwd = 5)
      lines(lowess(a~b), col="blue", lwd=5)
    } else if(input$variables1 == "Power/UI/w"){
      a = theData$power
      b = theData$msSinceStart
      plot(b,a, type = "o", col = "black", ylim=range(0, 500),
           xlab = "Time/ms", ylab = "Power/w",
           main = "Graph of Power")
      abline(h = 250, col = "red", lwd = 5)
      lines(lowess(a~b), col="blue", lwd=5)
    } else if(input$variables1 == "Voltage/U/V"){
      a = theData$voltage
      b = theData$msSinceStart
      plot(b,a, type = "o", col = "black", ylim=range(15, 25),
           xlab = "Time/ms", ylab = "Voltage/V",
           main = "Graph of Voltage")
      abline(h = 20, col = "red", lwd = 5)
      lines(lowess(a~b), col="blue", lwd=5)
    } else if(input$variables1 == "Energy Usage/%"){
      a = theData$energyLeft
      b = theData$msSinceStart
      plot(b,a, type = "o", col = "black", ylim=range(0,1),
           xlab = "Time/ms", ylab = "EnergyLeft",
           main = "Graph of Energy")
      abline(h = 0.05, col = "red", lwd = 75)
      abline(h = tail(theData, 1)$energyLeft, col = "red")
    } else if(input$variables1 == "Comprehensive"){
      layout(matrix(c(1,1,3,2,2,3), 2, 3, byrow = TRUE))
      a = theData$speed
      b = theData$msSinceStart
      plot(b,a, type = "o", col = "black", ylim=range(0, 30),
           xlab = "Time/ms", ylab = "Speed/mph",
           main = "Graph of Speed")
      abline(h = 15, col = "red", lwd = 5)
      lines(lowess(a~b), col="blue", lwd=5)
      a = theData$avgPower
      b = theData$msSinceStart
      c = theData$power
      plot(b,c, type = "o", col = "black", ylim=range(0, 500),
           xlab = "Time/ms", ylab = "Power/w",
           main = "Graph of Power")
      abline(h = 250, col = "red", lwd = 5)
      lines(lowess(a~b), col="blue", lwd=5)
      a = theData$energyLeft
      b = theData$msSinceStart
      plot(b,a, type = "o", col = "black", ylim=range(0,1),
           xlab = "Time/ms", ylab = "EnergyLeft",
           main = "Graph of Energy")
      abline(h = 0.05, col = "red", lwd = 75)
      abline(h = tail(theData, 1)$energyLeft, col = "red")
    }
  })
  
  
  output$distTable = renderTable({
    autoInvalidate()
    anotherdata = read.csv("allData.csv")
    data.frame(Speed = mean(anotherdata$speed), 
               Current = mean(anotherdata$current),
               Voltage = mean(anotherdata$voltage),
               Power = mean(anotherdata$power),
               Energy = mean(anotherdata$energyLeft)*100,
               Potential = mean(anotherdata$energyLeft) * 4000 * 2,
               Temprature = as.character(anotherdata$temprature) + "/40")
  })
})
