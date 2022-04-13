pipeline {
  agent any
  environment {
  }
  stages {
    stage('Build') {
              agent {
        label 'PlatformIO-slave'
      }
      steps {
        sh '''/home/pi/.local/bin/pio run'''
      }
    }

  
    stage('Hardware test') {
      agent {
        label 'PlatformIO-slave'
      }
      steps {
        sh '''/home/pi/.local/bin/pio run -e nanoatmega328 -t upload --upload-port /dev/ttyUSB0
sleep 5
python test_scripts/check.py'''
      }
    }
  }
}
