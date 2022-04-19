pipeline {
  agent none
  
  stages {
    stage('Build') {
              agent {
        label 'BuildNode'
      }
      steps {
        sh '''/home/pi/.local/bin/pio run'''
      }
    }

  
    stage('Hardware test') {
      agent {
        label 'TestNode'
      }
      steps {
        sh '''/home/pi/.local/bin/pio run -e nanoatmega328 -t upload --upload-port /dev/ttyUSB0
sleep 5
python test_scripts/check.py'''
      }
    }
  }
}
