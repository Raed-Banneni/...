# Generated by Django 4.1.7 on 2023-04-16 22:23

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('authenticate', '0004_date_delete_data'),
    ]

    operations = [
        migrations.CreateModel(
            name='historique',
            fields=[
                ('id', models.AutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('name', models.CharField(max_length=255)),
                ('moment_de_chute', models.DateTimeField(auto_now_add=True)),
            ],
        ),
        migrations.DeleteModel(
            name='date',
        ),
    ]