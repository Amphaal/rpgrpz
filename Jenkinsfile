pipeline {
    agent {
        docker { image 'amphaal/rpgrpz-ci-windows' }
    }
    stages {
        stage('Checkout') {
            steps {
                sh 'git clone --recursive-modules https://github.com/Amphaal/rpgrpz.git && cd rpgrpz'
            }
        }
        stage('Compile') {
            steps {
                sh 'cmake -GNinja -B_genRelease -H. -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/windows-ci.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo'
            }
        }
    }
}