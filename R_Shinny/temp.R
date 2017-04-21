
else if(input$variables1 == "ALL"){
  plot1 = ggplot(tail(data, input$timeLength), aes(x = time, y = speed)) +
    geom_point(alpha = 0.5) + geom_smooth(method = loess) + geom_line(alpha = 0.5) +
    xlab("Time Interval/ms") + ylab("Speed/mph")
  plot2 = ggplot(tail(data, input$timeLength), aes(x = time, y = current)) +
    geom_point(alpha = 0.5) + geom_smooth(method = loess) + geom_line(alpha = 0.5) +
    ylim(0, 3) + xlab("Time Interval/ms") + ylab("Current/A")
  plot3 = ggplot(tail(data, input$timeLength), aes(x = time, y = power)) +
    geom_point(alpha = 0.5) + geom_smooth(method = loess) + geom_line(alpha = 0.5) +
    ylim(0, 40) + xlab("Time Interval/ms") + ylab("Power/w")
  plot4 = ggplot(tail(data, input$timeLength), aes(x = time, y = voltage)) +
    geom_point(alpha = 0.5) + geom_smooth(method = loess) + geom_line(alpha = 0.5) +
    ylim(10, 20) + xlab("Time Interval/ms") + ylab("Voltage/V")
  grid.arrange(plot1, plot2, plot3, plot4, ncol = 2)
}