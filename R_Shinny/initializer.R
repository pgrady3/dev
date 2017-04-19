data = data.frame(sessionName = NA,
                  timeStamp = NA,
                  longtitude = 0,
                  latitude = 0,
                  voltage = 0,
                  current = 0,
                  speed = 0,
                  mileage = 0,
                  heading = 0,
                  msSinceStart = 0,
                  power = 0, 
                  avgPower = 0,
                  energyUsed = 0,
                  energyLeft = 1,
                  bateryStatus = "OK")
write.csv(data, file = "DEV_RacePenal/data.csv", row.names = FALSE)
write.csv(data, file = "DEV_RacePenal/allData.csv", row.names = FALSE)
write(NULL, file = "DEV_RacePenal/JSon.txt")