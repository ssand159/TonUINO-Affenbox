pipeline {
  agent any
  environment {

    BUILD_TOKEN = credentials('e18aa8c0-221e-4417-90b1-9b27c5268801')

  }
  stages {
    stage('Build') {
              agent {
        label 'PlatformIO-slave'
      }
      steps {
        sh '''/home/pi/.local/bin/pio run -r'''
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
