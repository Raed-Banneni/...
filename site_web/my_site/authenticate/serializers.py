from rest_framework import serializers
from .models import HeartRate

class HeartRateSerializer(serializers.ModelSerializer):
    class Meta:
        model = HeartRate
        fields = ['bpm', 'timestamp','famille','spo2']
        
    